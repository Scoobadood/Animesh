//
// Created by Dave Durbin (Old) on 4/10/21.
//

#include "MultiResolutionSurfelGraph.h"
#include "SurfelGraph.h"
#include "SurfelBuilder.h"
#include <spdlog/spdlog.h>
#include <string>
#include <map>
#include <Eigen/Core>

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

  sb->
      with_id(new_id)->
      with_tangent(new_tangent);
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
    dual_area_by_node.emplace(node->data()->id(), 1);
  }

  // Compute mean normal for each vertex
  map<string, Eigen::Vector3f> mean_normal_by_node;
  for (const auto &node: m_levels.back()->nodes()) {
    auto mean_normal = compute_mean_normal(node);
    mean_normal_by_node[node->data()->id()] = mean_normal;
  }


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
  map<SurfelGraph::Edge, float, SurfelGraphEdgeComparator> scores;
  compute_scores(m_levels.back(), dual_area_by_node, mean_normal_by_node, scores);
  vector<pair<SurfelGraph::Edge, float>> edges_and_scores;
  edges_and_scores.reserve(scores.size());
  for (auto &edge_and_score: scores) {
    edges_and_scores.emplace_back(edge_and_score);
  }
  sort(edges_and_scores.begin(),
       edges_and_scores.end(),
       [&](const pair<SurfelGraph::Edge, float> &a, const pair<SurfelGraph::Edge, float> &b) {
         return a.second > b.second;
       }
  );

  SurfelBuilder sb{m_random_engine};
  // Map from each node in this level to  node in the next level.
  map<SurfelGraphNodePtr, SurfelGraphNodePtr> fine_to_coarse_mapping;

  // Map from each node in the upper level to the parents in the lower level
  map<SurfelGraphNodePtr, pair<SurfelGraphNodePtr, SurfelGraphNodePtr>> coarse_to_fine_mapping;

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
    const auto &first_node = edge_and_score.first.from();
    const auto &second_node = edge_and_score.first.to();
    const auto &first_node_id = first_node->data()->id();
    const auto &second_node_id = second_node->data()->id();

    // Only collapse if neither node has already been involved in a collapse
    if ((fine_to_coarse_mapping.count(first_node) > 0) || (fine_to_coarse_mapping.count(second_node) > 0)) {
      continue;
    }

    // Make a new node
    auto new_surfel = surfel_merge_function(first_node->data(),
                                            (float) dual_area_by_node.at(first_node_id),
                                            second_node->data(),
                                            (float) dual_area_by_node.at(second_node_id));
    auto new_node = new_graph->add_node(new_surfel);
    fine_to_coarse_mapping.emplace(first_node, new_node);
    fine_to_coarse_mapping.emplace(second_node, new_node);
    coarse_to_fine_mapping.emplace(new_node, make_pair<>(first_node, second_node));

    // Update the dual areas
    auto val = dual_area_by_node.at(first_node_id) + dual_area_by_node.at(second_node_id);
    dual_area_by_node.erase(first_node_id);
    dual_area_by_node.erase(second_node_id);
    dual_area_by_node.insert({new_node->data()->id(), val});
  } // End of collapsing edges


  // Copy uncollapsed nodes directly.
  for (const auto &node: m_levels.back()->nodes()) {
    // Ignore nodes that are mapped
    if (fine_to_coarse_mapping.count(node) > 0) {
      continue;
    }

    // Make a new node, copying the existing surfel.
    auto s = make_shared<Surfel>(sb.with_surfel(node->data())->build());
    auto new_node = new_graph->add_node(s);
    fine_to_coarse_mapping.emplace(node, new_node);
    coarse_to_fine_mapping.emplace(new_node, make_pair<>(node, nullptr));
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
    const auto it = fine_to_coarse_mapping.find(n);
    // Orphan. complain - shouldn;t happend any more
    if (it == fine_to_coarse_mapping.end()) {
      spdlog::warn("Unexpected orpan");
    }

    const auto &N = it->second;
    const auto neighours = m_levels.back()->neighbours(n, true);
    for (const auto &nn: neighours) {
      const auto it_nn = fine_to_coarse_mapping.find(nn);
      // Orphan. Ignore.
      if (it_nn == fine_to_coarse_mapping.end()) {
        spdlog::warn("Unexpected orpan");
      }
      const auto &NN = it_nn->second;
      // No selfies.
      if (N == NN) {
        continue;
      }
      if (new_graph->has_edge(N, NN)) {
        continue;
      }
      new_graph->add_edge(N, NN, SurfelGraphEdge{1.0f});
    }
  }

  m_up_mapping.push_back(coarse_to_fine_mapping);
  m_levels.push_back(new_graph);
}

void
fix_value(float &value) {
  while (value >= 0.5f) {
    value -= 1.0f;
  }
  while (value < -0.5f) {
    value += 1.0f;
  }
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
      if (parents.second == nullptr) {
        spdlog::debug("Found single parent for node {} in level {}: {}", node->data()->id(), from_level,
                      parents.first->data()->id());
      } else {
        spdlog::debug("Found parents for node {} in level {}: {} and {}", node->data()->id(), from_level,
                      parents.first->data()->id(),
                      parents.second->data()->id());
      }

      if (rosy) {
        parents.first->data()->setTangent(node->data()->tangent());
        if (parents.second != nullptr) {
          parents.second->data()->setTangent(node->data()->tangent());
        }
      }

      if (posy) {
        if (parents.second == nullptr) {
          // Directly propagate orphan stuff down.
          parents.first->data()->set_reference_lattice_offset(node->data()->reference_lattice_offset());
        } else {
          // Where there are two parents, we get them to agree on a CLP
          // Get common frames for parents
          std::set<unsigned int> s1_frames, s2_frames;
          s1_frames.insert(begin(parents.first->data()->frames()), end(parents.first->data()->frames()));
          s2_frames.insert(begin(parents.second->data()->frames()), end(parents.second->data()->frames()));
          std::vector<unsigned int> shared_frames(std::min(s1_frames.size(), s2_frames.size()));
          auto it = set_intersection(s1_frames.begin(), s1_frames.end(),
                                     s2_frames.begin(), s2_frames.end(),
                                     shared_frames.begin());
          shared_frames.resize(it - shared_frames.begin());

          // Use position in first shared frame to manage
          const auto reference_frame = *shared_frames.begin();
          Eigen::Vector3f ref_position, ref_u, ref_v, ref_n, ref_lattice_position;
          node->data()->get_all_data_for_surfel_in_frame(reference_frame,
                                                         ref_position,
                                                         ref_u,
                                                         ref_v,
                                                         ref_n,
                                                         ref_lattice_position);

          Eigen::Vector3f p1_position, p1_t, p1_n;
          parents.first->data()->get_vertex_tangent_normal_for_frame(reference_frame, p1_position, p1_t, p1_n);
          Eigen::Vector3f p2_position, p2_t, p2_n;
          parents.second->data()->get_vertex_tangent_normal_for_frame(reference_frame, p2_position, p2_t, p2_n);

          // TODO Fix these to account for RHO and handle values that wind outisde [-0.5, 0.5)* Rho
          Eigen::Vector3f p1_rel_position = ref_lattice_position - p1_position;
          p1_rel_position -= (p1_n * p1_n.dot(p1_rel_position));
          auto p1_u = p1_rel_position.dot(p1_t);
          auto p1_v = p1_rel_position.dot(p1_n.cross(p1_t));

          Eigen::Vector3f p2_rel_position = ref_lattice_position - p2_position;
          p2_rel_position -= (p2_n * p2_n.dot(p2_rel_position));
          auto p2_u = p2_rel_position.dot(p2_t);
          auto p2_v = p2_rel_position.dot(p2_n.cross(p2_t));

          parents.first->data()->set_reference_lattice_offset({p1_u, p1_v});
          parents.second->data()->set_reference_lattice_offset({p2_u, p2_v});
          fix_value(p1_u);
          fix_value(p2_u);
          fix_value(p1_v);
          fix_value(p2_v);

          if (fabsf(p1_u) > 0.5f ||
              fabsf(p1_v) > 0.5f ||
              fabsf(p2_u) > 0.5f ||
              fabsf(p2_v) > 0.5f
              ) {
            spdlog::warn("One of these p1=({:3f}, {:3f}), p2=({:3f}, {:3f}) is out of range propagating from {}",
                         p1_u, p1_v, p2_u, p2_v, from_level
            );
          }
        }
      }
    }
  }
}
