//
// Created by Dave Durbin (Old) on 4/10/21.
//

#include "MultiResolutionSurfelGraph.h"
#include "SurfelGraph.h"
#include "SurfelBuilder.h"
#include <spdlog/spdlog.h>
#include <string>
#include <map>

struct MultiResolutionSurfelGraph::SurfelGraphEdgeComparator {
  bool operator()(const SurfelGraph::Edge &lhs, const SurfelGraph::Edge &rhs) const {
    if (lhs.from()->data()->id() < rhs.from()->data()->id()) {
      return true;
    }
    if (lhs.from()->data()->id() > rhs.from()->data()->id()) {
      return false;
    }
    return (lhs.to()->data()->id() < rhs.to()->data()->id());
  }
};

std::shared_ptr<Surfel>
MultiResolutionSurfelGraph::surfel_merge_function(
    const std::shared_ptr<Surfel> &n1,
    float w1,
    const std::shared_ptr<Surfel> &n2,
    float w2) {
  using namespace std;

  set<unsigned int> n1_frames;
  n1_frames.insert(n1->frames().begin(), n1->frames().end());
  set<unsigned int> n2_frames;
  n2_frames.insert(n2->frames().begin(), n2->frames().end());
  vector<unsigned int> common_frames(min(n1_frames.size(), n2_frames.size()));
  auto it = set_intersection(n1_frames.begin(), n1_frames.end(),
                             n2_frames.begin(), n2_frames.end(),
                             common_frames.begin());
  common_frames.resize(it - common_frames.begin());
  assert (!common_frames.empty());

  auto *sb = new SurfelBuilder(m_random_engine);
  auto new_id = "(" + n1->id() + " + " + n2->id() + ")";
  auto new_tangent = ((n1->tangent() * w1) + (n2->tangent() * w2)).normalized();
  auto new_offset = ((n1->reference_lattice_offset() * w1) + (n2->reference_lattice_offset() * w2)) / (w1 + w2);
  sb->with_id(new_id)
      ->with_tangent(new_tangent)
      ->with_reference_lattice_offset(new_offset);
  for (const auto frame: common_frames) {
    Eigen::Vector3f vert1, tan1, norm1;
    Eigen::Vector3f vert2, tan2, norm2;
    n1->get_vertex_tangent_normal_for_frame(frame, vert1, tan1, norm1);
    n2->get_vertex_tangent_normal_for_frame(frame, vert2, tan2, norm2);
    auto frame_norm = (norm1 * w1 + norm2 * w2).normalized();
    auto frame_pos = ((vert1 * w1) + (vert2 * w2)) / (w1 + w2);
    sb->with_frame(
        {0, 0, frame},
        0.0f,
        frame_norm,
        frame_pos);
  }
  return make_shared<Surfel>(sb->build());
}

SurfelGraphEdge
edge_merge_function(const SurfelGraphEdge &e1, float w1, const SurfelGraphEdge &e2, float w2) {
  return SurfelGraphEdge{
      ((e1.weight() * w1) + (e2.weight() * w2)) / (w1 + w2)
  };
};

MultiResolutionSurfelGraph::MultiResolutionSurfelGraph(
    const SurfelGraphPtr &surfel_graph,
    std::default_random_engine &rng)
    : m_random_engine{rng} //
{
  m_levels.push_back(surfel_graph);
}

/*
 * Generate levels (if not already done).
 */
void
MultiResolutionSurfelGraph::generate_levels(unsigned int num_levels) {
  spdlog::debug("generate_levels({})", num_levels);

  if (num_levels == 0) {
    spdlog::error("  Can't generate 0 levels");
    return;
  }

  if (num_levels > 10) {
    spdlog::warn("  Seems like a lot of levels");
  }

  if (m_levels.size() >= num_levels) {
    return;
  }

  for (unsigned int lvl_idx = 1; lvl_idx < num_levels; ++lvl_idx) {
//    generate_new_level();
    generate_new_level_additive();
  }
}

/**
 * Compute the mean normal for the given node across
 * all frames in which it appears.
 */
Eigen::Vector3f
MultiResolutionSurfelGraph::compute_mean_normal(const SurfelGraphNodePtr &graph_node_ptr) {
  Eigen::Vector3f normal{0, 0, 0};

  for (const auto &frame: graph_node_ptr->data()->frame_data()) {
    normal += frame.normal;
  }
  return normal.normalized();
}

/**
 * Compute the scores for each edge.
 */
void
MultiResolutionSurfelGraph::compute_scores(
    const SurfelGraphPtr &current_graph,
    const std::map<std::string, int> &dual_area_by_node,
    const std::map<std::string, Eigen::Vector3f> &mean_normal_by_node,
    std::map<SurfelGraph::Edge, float, SurfelGraphEdgeComparator> &scores) {

  for (const auto &edge: current_graph->edges()) {
    auto from_id = edge.from()->data()->id();
    auto to_id = edge.to()->data()->id();
    auto key = make_pair(from_id, to_id);
    auto a = (float) dual_area_by_node.at(from_id) / (float) dual_area_by_node.at(to_id);
    if (a > 1.0f) {
      a = 1.0f / a;
    }
    auto dp = mean_normal_by_node.at(from_id).dot(mean_normal_by_node.at(to_id));
    scores[edge] = a * dp;
  }
}

/**
 * Generate the next level for this multi-resolution graph.
 * Uses an additive approach
 */
void
MultiResolutionSurfelGraph::generate_new_level_additive() {
  using namespace std;

  /*
   *  Preprocessing:
   *  assign a dual area Ai to each vertex i (uniform Ai = 1, or Voronoi area when the input is a mesh).
   */
  map<string, int> dual_area_by_node;
  for (const auto &node: m_levels.back()->nodes()) {
    dual_area_by_node.insert({node->data()->id(), 1});
  }

  // Compute mean normal for each vertex
  map<string, Eigen::Vector3f> mean_normal_by_node;
  for (const auto &node: m_levels.back()->nodes()) {
    auto mean_normal = compute_mean_normal(node);
    mean_normal_by_node[node->data()->id()] = mean_normal;
  }
  map<SurfelGraph::Edge, float, SurfelGraphEdgeComparator> scores;


  /*
    2. Repeat the following phases until a fixed point is reached:
      (a) For each pair of neighboring vertices i,j,
          assign a score Sij := ⟨ni,nj⟩min(Ai/Aj, Aj/Ai).
      (b) Traverse Sij in decreasing order and collapse vertices
          i and j into a new vertex v if neither has been involved
          in a collapse operation thus far. The new vertex is assigned
          an area of Av = Ai + Aj , and its other properties are
          given by area-weighted averages of the vertices that are
          merged together.
   */
  compute_scores(m_levels.back(), dual_area_by_node, mean_normal_by_node, scores);
  vector<pair<SurfelGraph::Edge, float>> edges_and_scores;
  edges_and_scores.reserve(scores.size());
  for (auto &edge_and_score: scores) {
    edges_and_scores.emplace_back(edge_and_score);
  }
  sort(edges_and_scores.begin(),
       edges_and_scores.end(),
       [=](pair<SurfelGraph::Edge, float> &a, pair<SurfelGraph::Edge, float> &b) {
         return a.second > b.second;
       }
  );

  SurfelBuilder sb{m_random_engine};
  map<SurfelGraphNodePtr, SurfelGraphNodePtr> lower_to_upper_mapping;
  map<SurfelGraphNodePtr, pair<SurfelGraphNodePtr, SurfelGraphNodePtr>> upper_to_lower_mapping;

  SurfelGraphPtr new_graph = make_shared<SurfelGraph>();
  for (const auto &edge_and_score: edges_and_scores) {

    /*
     * for edge E in G
     *    n1, n2 = nodes of E
     *    if n1 has collapsed or n2 has collapsed
     *       continue
     *    make a new node N = n1+n2
     *    add N to G'
     *    record that n1 and n2 ==> N
     * end
     */
    const auto first_node = edge_and_score.first.from();
    const auto second_node = edge_and_score.first.to();
    const auto first_node_id = first_node->data()->id();
    const auto second_node_id = second_node->data()->id();

    // Only collapse if neither node has already been involved in a collapse
    if (lower_to_upper_mapping.count(first_node) > 0) {
      continue;
    }
    if (lower_to_upper_mapping.count(second_node) > 0) {
      continue;
    }

    // Make a new node
    auto new_surfel = surfel_merge_function(first_node->data(),
                                            (float) dual_area_by_node.at(first_node_id),
                                            second_node->data(),
                                            (float) dual_area_by_node.at(second_node_id));
    auto new_node = new_graph->add_node(new_surfel);
    lower_to_upper_mapping.emplace(first_node, new_node);
    lower_to_upper_mapping.emplace(second_node, new_node);
    upper_to_lower_mapping.emplace(new_node, make_pair<>(first_node, second_node));

    // Update the dual areas
    auto val = dual_area_by_node.at(first_node_id) + dual_area_by_node.at(second_node_id);
    dual_area_by_node.erase(first_node_id);
    dual_area_by_node.erase(second_node_id);
    dual_area_by_node.insert({new_node->data()->id(), val});
  } // End of collapsing edges

  /*
   * for any node n that didn't collapse
   *   make a new node N = n
   *   add N to G'
   *   record that n1 ==> N
   * end
   */
  for (const auto &n: m_levels.back()->nodes()) {
    if (lower_to_upper_mapping.count(n) > 0) {
      continue;
    }
    auto new_node = new_graph->add_node(n->data());
    lower_to_upper_mapping.emplace(n, new_node);
    upper_to_lower_mapping.emplace(new_node, make_pair<>(n, n));
  }

  /*
   * For each node n in G
   *   find N in G' corresponding to n
   *   w1 = dual_area  for N
   *   for each neighbour nn in nbr(n)
   *     find NN in G' corresponding to nn
   *     w2 = dual_area for NN
   *
   *     add edge N, NN to G' with weight
   *   end
   * end
   */
  for (const auto &n: m_levels.back()->nodes()) {
    const auto &N = lower_to_upper_mapping.at(n);
    const auto neighours = m_levels.back()->neighbours(n, true);
    for (const auto &nn: neighours) {
      const auto &NN = lower_to_upper_mapping.at(nn);
      // eliminate 'self' edges
      if( N == NN ) {
        continue;
      }
      if (new_graph->has_edge(N, NN)) {
        continue;
      }
      new_graph->add_edge(N, NN, SurfelGraphEdge{1.0f});
    }
  }

  m_up_mapping.push_back(upper_to_lower_mapping);
  m_levels.push_back(new_graph);
}

/**
 * Up-propagate data from one level to the next.
 */
void
MultiResolutionSurfelGraph::propagate(
    unsigned int from_level //
    , bool rosy //
    , bool posy //
) {
  assert(from_level <= m_up_mapping.size());
  assert(from_level > 0);
  // For each node in the from_level graph,
  for (const auto &node: m_levels[from_level]->nodes()) {
    auto &up_mapping = m_up_mapping[from_level - 1];
    auto mapping = up_mapping.find(node);
    if (mapping == end(m_up_mapping[from_level - 1])) {
      spdlog::error("Didn't find up mapping for node {} in level {}", node->data()->id(), from_level);
    } else {
      auto parents = mapping->second;
      // Find the nodes in the graph
      spdlog::info("Found parents for node {} in level {}: {} and {}", node->data()->id(), from_level,
                   parents.first->data()->id(),
                   parents.second->data()->id());
      if(rosy) {
        parents.first->data()->setTangent(node->data()->tangent());
        parents.second->data()->setTangent(node->data()->tangent());
      }

      if(posy) {
        //TODO: Make this better
        // Right now we up propagate the same offset to both nodes
        // A better approach would perhaps be to compute the location of this node in 3 space and then
        // calculate an *actual* offset from the vertices across a single frame or averaged across all frames
        parents.first->data()->set_reference_lattice_offset(node->data()->reference_lattice_offset());
        parents.second->data()->set_reference_lattice_offset(node->data()->reference_lattice_offset());
      }
    }
  }
}
