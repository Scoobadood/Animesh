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
    std::mt19937 &rng)
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
    generate_new_level();
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
 */
void
MultiResolutionSurfelGraph::generate_new_level() {
  using namespace std;

  SurfelGraphPtr new_graph = make_shared<SurfelGraph>(*m_levels.back());
  /*
   *  Preprocessing:
   *  assign a dual area Ai to each vertex i (uniform Ai = 1, or Voronoi area when the input is a mesh).
   */
  map<string, int> dual_area_by_node;
  for (const auto &node: new_graph->nodes()) {
    dual_area_by_node.insert({node->data()->id(), 1});
  }

  // Compute mean normal for each vertex
  map<string, Eigen::Vector3f> mean_normal_by_node;
  for (const auto &node: new_graph->nodes()) {
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
  compute_scores(new_graph, dual_area_by_node, mean_normal_by_node, scores);
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
  set<string> collapsed;

  map<SurfelGraphNodePtr, pair<SurfelGraphNodePtr, SurfelGraphNodePtr>> level_mapping;
  for (const auto &edge_and_score: edges_and_scores) {
    const auto first_node = edge_and_score.first.from();
    const auto second_node = edge_and_score.first.to();
    const auto first_node_id = first_node->data()->id();
    const auto second_node_id = second_node->data()->id();

    // Only collapse if neither node has already been involved in a collapse
    if (collapsed.count(first_node_id) > 0 ||
        collapsed.count(second_node_id) > 0) {
      continue;
    }

    // Perform the collapse and remember it.
    auto new_node = new_graph->collapse_edge(
        first_node, second_node,
        [&](const std::shared_ptr<Surfel> &n1,
            float w1,
            const std::shared_ptr<Surfel> &n2,
            float w2) {
          return surfel_merge_function(n1, w1, n2, w2);
        },
        ::edge_merge_function,
        (float) dual_area_by_node.at(first_node_id),
        (float) dual_area_by_node.at(second_node_id));
    collapsed.emplace(first_node_id);
    collapsed.emplace(second_node_id);

    // Update the dual areas
    auto val = dual_area_by_node.at(first_node_id) + dual_area_by_node.at(second_node_id);
    dual_area_by_node.erase(first_node_id);
    dual_area_by_node.erase(second_node_id);
    dual_area_by_node.insert({new_node->data()->id(), val});

    // Update the node mapping table
    level_mapping.insert({new_node, {first_node, second_node}});
  }

  m_up_mapping.push_back(level_mapping);
  m_levels.push_back(new_graph);
}

/**
 * Up-propagate data from one level to the next.
 */
void
MultiResolutionSurfelGraph::propagate(unsigned int from_level) {
  assert(from_level <= m_up_mapping.size());
  assert(from_level > 0);
  // For each node in the from_level graph,
  for (const auto &node: m_levels[from_level]->nodes()) {
    auto mapping = m_up_mapping[from_level - 1].find(node);
    if (mapping == end(m_up_mapping[from_level - 1])) {
      // Not found
      for (auto &parent_node: m_levels[from_level - 1]->nodes()) {
        if (parent_node->data()->id() == node->data()->id()) {
          parent_node->data()->setTangent(node->data()->tangent());
          parent_node->data()->set_reference_lattice_offset(node->data()->reference_lattice_offset());
          break;
        }
      }
    } else {
      auto parents = mapping->second;
      // Find the nodes in the graph
      parents.first->data()->setTangent(node->data()->tangent());
      parents.second->data()->setTangent(node->data()->tangent());

      //TODO: Make this better
      // Right now we up propagate the same offset to both nodes
      // A better approach would perhaps be to compute the location of this node in 3 space and then
      // calculate an *actual* offset from the vertices across a single frame or averaged across all frames
      parents.first->data()->set_reference_lattice_offset(node->data()->reference_lattice_offset());
      parents.second->data()->set_reference_lattice_offset(node->data()->reference_lattice_offset());
    }
  }
}
