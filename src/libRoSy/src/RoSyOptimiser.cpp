#include "RoSy/RoSyOptimiser.h"

#include <set>
#include <vector>
#include <algorithm>
#include <RoSy/RoSy.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>
#include <Vote/VoteCounter.h>
#include <strstream>

RoSyOptimiser::RoSyOptimiser(const Properties &properties)
    : Optimiser(properties) //
{
  setup_termination_criteria(
      "rosy-termination-criteria",
      "rosy-term-crit-relative-smoothness",
      "rosy-term-crit-absolute-smoothness",
      "rosy-term-crit-max-iterations");

  m_damping_factor = m_properties.getFloatProperty("rosy-damping-factor");
  m_weight_for_error = m_properties.getBooleanProperty("rosy-weight-for-error");
  m_weight_for_error_steps = m_properties.getIntProperty("rosy-weight-for-error-steps");
  m_randomise_neighour_order = m_properties.getBooleanProperty("rosy-randomise-neighbour-order");
}

/**
 * @return The mean error per neighbour.
 */
float
RoSyOptimiser::compute_node_smoothness_for_frame(
    const SurfelGraphNodePtr &this_node,
    size_t frame_index,
    unsigned int &num_neighbours,
    bool is_first_run) const {

  using namespace Eigen;

  float frame_smoothness = 0.0f;

  const auto &this_surfel = this_node->data();
  spdlog::debug("Computing smoothness for node {} in frame {}", this_surfel->id(), frame_index);

  Vector3f vertex, normal, tangent;
  this_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

  // For each neighbour in frame...
  const auto neighbours_in_frame = get_node_neighbours_in_frame(m_surfel_graph, this_node, frame_index);
  for (const auto &nbr_node: neighbours_in_frame) {
    const auto &nbr_surfel = nbr_node->data();

    Vector3f nbr_vertex, nbr_normal, nbr_tangent;
    nbr_surfel->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

    // Get k_ij and k_ji from edge
    auto k = get_k(m_surfel_graph, this_node, nbr_node);
    auto v1 = vector_by_rotating_around_n(tangent, normal, k.first);
    auto v2 = vector_by_rotating_around_n(nbr_tangent, nbr_normal, k.second);

    // Compute k_ij and k_ji
    unsigned short k_ij_computed, k_ji_computed;
    auto best_pair = best_rosy_vector_pair(tangent, normal, k_ij_computed, nbr_tangent, nbr_normal, k_ji_computed);
    float theta_computed = degrees_angle_between_vectors(best_pair.first, best_pair.second);

    float theta = degrees_angle_between_vectors(v1, v2);

    if (this_surfel->id() == "v_1497" || nbr_surfel->id() == "v_1497") {
      if (k.first != k_ij_computed || k.second != k_ji_computed) {
        spdlog::warn("{}->{}:  ({}, {}) doesn't match computed ({}, {}) theta:{}, computed theta:{}",
                     this_surfel->id(),
                     nbr_surfel->id(),
                     k.first, k.second,
                     k_ij_computed, k_ji_computed,
                     theta, theta_computed
        );
      } else {
        spdlog::info("{}->{}:  ({}, {}) matches computed score. theta:{}",
                     this_surfel->id(),
                     nbr_surfel->id(),
                     k.first, k.second,
                     theta
        );
      }
    }
    const auto smoothness = (theta * theta);
    frame_smoothness += smoothness;
  }

  num_neighbours = neighbours_in_frame.size();
  spdlog::debug(" smoothness {:4.3f}", frame_smoothness);
  return frame_smoothness;
}

bool
RoSyOptimiser::compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const {
  // Return true if l has higher smoothness : ie is less smooth
  return l->data()->rosy_smoothness() > r->data()->rosy_smoothness();
}

void
RoSyOptimiser::adjust_weights_based_on_error(const std::shared_ptr<Surfel> &s1,
                                             const std::shared_ptr<Surfel> &s2,
                                             float &w_ij,
                                             float &w_ji) const {
  using namespace std;

  static int iterations = 0;

  float smooth1 = s1->rosy_smoothness();
  float smooth2 = s2->rosy_smoothness();

  const auto total_smoothness = smooth1 + smooth2;
  if (total_smoothness != 0) {
    float delta = (smooth1 - smooth2) / total_smoothness;
    // If delta > 0 we want to weight ij less than ji
    delta *= (iterations / (float) m_weight_for_error_steps);
    delta = fminf(0.9f, fmaxf(-0.9f, delta));
    w_ij -= delta;
    w_ji += delta;
  }
  ++iterations;
}

/*
 * Retrieve the set of frame indices for common frames of two nodes
 */
std::vector<unsigned int>
RoSyOptimiser::get_common_frames(
    const std::shared_ptr<Surfel> &s1,
    const std::shared_ptr<Surfel> &s2
) {
  using namespace std;

  set<unsigned int> s1_frames, s2_frames;
  s1_frames.insert(begin(s1->frames()), end(s1->frames()));
  s2_frames.insert(begin(s2->frames()), end(s2->frames()));
  vector<unsigned int> shared_frames(min(s1_frames.size(), s2_frames.size()));
  auto it = set_intersection(s1_frames.begin(), s1_frames.end(),
                             s2_frames.begin(), s2_frames.end(),
                             shared_frames.begin());
  shared_frames.resize(it - shared_frames.begin());
  return shared_frames;
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
RoSyOptimiser::optimise_node_with_voting(const SurfelGraphNodePtr &this_node) {
  using namespace std;
  using namespace Eigen;

  const auto &this_surfel = this_node->data();
  const auto &old_tangent = this_surfel->tangent();
  Vector3f new_tangent{old_tangent};

  // For each neighbour of this surfel
  float w_sum = 0.0f;
  auto neighbours = m_surfel_graph->neighbours(this_node);
  for (const auto &nbr: neighbours) {
    const auto &nbr_surfel = nbr->data();

    // Get the set of common frames
    auto shared_frames = get_common_frames(this_surfel, nbr_surfel);

    // Initialise vote
    VoteCounter<pair<unsigned short, unsigned short>> vote_counter{
        [&](const vector<pair<unsigned short, unsigned short>> &possibilities) -> pair<int, int> {
          uniform_int_distribution <size_t> i(0, possibilities.size() - 1);
          return possibilities[i(m_random_engine)];
        }
    };

    // For each frame that they both appear in
    for (unsigned int frame_index: shared_frames) {
      // Compute best k_ij/k_ji and tally vote
      unsigned short k_ij, k_ji;
      Vector3f nbr_tan_in_surfel_space, nbr_norm_in_surfel_space;
      this_surfel->transform_surfel_via_frame(
          nbr_surfel, frame_index,
          nbr_norm_in_surfel_space,
          nbr_tan_in_surfel_space);
      best_rosy_vector_pair(
          new_tangent, Vector3f::UnitY(), k_ij,
          nbr_tan_in_surfel_space, nbr_norm_in_surfel_space, k_ji);
      vote_counter.cast_vote({k_ij, k_ji});
    }
    auto winner = vote_counter.winner();
    // DEBUG
    if (this_surfel->id() == "v_1497" || nbr_surfel->id() == "v_1497") {
      std::ostringstream s;
      s << this_surfel->id() << "->" << nbr_surfel->id() << ":";
      for (const auto &vc: vote_counter.votes_with_counts()) {
        s << "(" << std::to_string(vc.first.first)
          << ", "
          << std::to_string(vc.first.second)
          << ") [" << std::to_string(vc.second)
          << "] ";
      }
      s << "==> (" << winner.first << ", " << winner.second << ")";
      spdlog::info("{}", s.str());
    }
    // END DEBUG
    unsigned short k_ij = winner.first;
    unsigned short k_ji = winner.second;

    // For each frame that they both appear in
    for (unsigned int frame_index: shared_frames) {
      float w_ij = 1.0f;
      float w_ji = 1.0f;

      Vector3f nbr_tan_in_surfel_space, nbr_norm_in_surfel_space;
      this_surfel->transform_surfel_via_frame(nbr_surfel, frame_index,
                                              nbr_norm_in_surfel_space,
                                              nbr_tan_in_surfel_space);

      // Compute smoothing
      auto v1 = vector_by_rotating_around_n(new_tangent, Vector3f::UnitY(), k_ij);
      auto v2 = vector_by_rotating_around_n(nbr_tan_in_surfel_space, nbr_norm_in_surfel_space, k_ji);
      Vector3f v = (v1 * w_sum) + (v2 * w_ji);
      w_sum += w_ij;
      new_tangent = project_vector_to_plane(v, Vector3f::UnitY());
    } // Next frame
    set_k(m_surfel_graph, this_node, k_ij, nbr, k_ji);
  } // Next neighbour
  this_node->data()->setTangent(new_tangent);
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &this_node) {
  using namespace Eigen;
  using namespace std;

  if (m_properties.getBooleanProperty("rosy-vote-for-best-k")) {
    optimise_node_with_voting(this_node);
    return;
  }

  const auto &this_surfel = this_node->data();
  const auto &old_tangent = this_surfel->tangent();
  Vector3f new_tangent{old_tangent};

  // For each frame that this surfel appears in.
  for (const auto frame_index: this_surfel->frames()) {
    Eigen::Vector3f vertex, normal, tangent;
    this_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

    float w_sum = 0.0f;
    auto neighbours_in_frame =
        get_neighbours_of_node_in_frame(m_surfel_graph, this_node, frame_index, m_randomise_neighour_order);
    for (const auto &neighbour_node: neighbours_in_frame) {

      const auto &nbr_surfel = neighbour_node->data();

      float w_ij = 1.0f;
      float w_ji = 1.0f;
      if (m_weight_for_error) {
        adjust_weights_based_on_error(this_surfel, nbr_surfel, w_ij, w_ji);
      }

      Vector3f neighbour_tan_in_surfel_space, neighbour_norm_in_surfel_space;
      this_surfel->transform_surfel_via_frame(
          nbr_surfel, frame_index,
          neighbour_norm_in_surfel_space,
          neighbour_tan_in_surfel_space);

      unsigned short k_ij;
      unsigned short k_ji;
      new_tangent = average_rosy_vectors(
          new_tangent,
          Vector3f::UnitY(),
          w_sum,
          neighbour_tan_in_surfel_space,
          neighbour_norm_in_surfel_space,
          w_ji,
          k_ij,
          k_ji);
      w_sum += w_ij;

      // Store ks
      set_k(m_surfel_graph, this_node, k_ij, neighbour_node, k_ji);
    }
  }

  if (m_damping_factor > 0) {
    new_tangent = (m_damping_factor * old_tangent) + ((1.0f - m_damping_factor) * new_tangent);
    new_tangent = project_vector_to_plane(new_tangent, Vector3f::UnitY(), true);
  }
  this_node->data()->setTangent(new_tangent);
  this_node->data()->set_rosy_correction(degrees_angle_between_vectors(old_tangent, new_tangent));
}

void RoSyOptimiser::store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const {
  node->data()->set_rosy_smoothness(smoothness);
}