#include "RoSy/RoSyOptimiser.h"

#include <set>
#include <vector>
#include <algorithm>
#include <RoSy/RoSy.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Vote/VoteCounter.h>
#include <spdlog/spdlog.h>

RoSyOptimiser::RoSyOptimiser(const Properties &properties, std::mt19937 &rng)
    : NodeOptimiser(properties, rng) //
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
  m_vote_for_best_k = m_properties.getBooleanProperty("rosy-vote-for-best-k");

  setup_ssa();
}

/**
 * @return The total smoothness for a given node in a frame.
 * Also sets the number of neighbours it's compared to so that the mean can be computed.
 */
float
RoSyOptimiser::compute_node_smoothness_for_frame(
    const SurfelGraphNodePtr &this_node,
    size_t frame_index,
    unsigned int &num_neighbours) const {

  using namespace Eigen;

  float frame_smoothness = 0.0f;

  const auto &this_surfel = this_node->data();

  bool should_dump = false;//(this_surfel->id() == "s_32");
  // START DEBUG
  if (should_dump) {
    spdlog::info("Computing smoothness for node {} in frame {}", this_surfel->id(), frame_index);
  }
  // END DEBUG

  Vector3f vertex, normal, tangent;
  this_surfel->get_vertex_tangent_normal_for_frame(frame_index, vertex, tangent, normal);

  // START DEBUG
  if (should_dump) {
    spdlog::info("  Stored tan is ({},{},{})",
                 this_surfel->tangent()[0], this_surfel->tangent()[1], this_surfel->tangent()[2]);
    spdlog::info("  In frame is ({},{},{})",
                 tangent[0], tangent[1], tangent[2]);
  }
  // END DEBUG

  int bad_count = 0, good_count = 0;
  // For each neighbour in frame...
  const auto neighbours_in_frame = get_node_neighbours_in_frame(m_surfel_graph, this_node, frame_index);
  for (const auto &nbr_node: neighbours_in_frame) {
    const auto &nbr_surfel = nbr_node->data();

    Vector3f nbr_vertex, nbr_normal, nbr_tangent;
    nbr_surfel->get_vertex_tangent_normal_for_frame(frame_index, nbr_vertex, nbr_tangent, nbr_normal);

    // Compute best Ks
    unsigned short k_ij;
    unsigned short k_ji;
    auto best_pair = best_rosy_vector_pair(tangent, normal, k_ij, nbr_tangent, nbr_normal, k_ji);

    float theta = degrees_angle_between_vectors(best_pair.first, best_pair.second);
    if (theta > 45 || theta < -45) {
      bad_count++;
    } else {
      good_count++;
    }
    if (should_dump) {
      spdlog::info("  Edge to ", nbr_surfel->id());
      spdlog::info("    k_ij = {},   k_ji = {}", k_ij, k_ji);
      spdlog::info("    V1=[{},{},{}]", vertex[0], vertex[1], vertex[2]);
      spdlog::info("    N1=[{}, {}, {}]", normal[0], normal[1], normal[2]);
      spdlog::info("    T1=[{}, {}, {}]", tangent[0], tangent[1], tangent[2]);
      spdlog::info("    V2=[{},{},{}]", nbr_vertex[0], nbr_vertex[1], nbr_vertex[2]);
      spdlog::info("    N2=[{}, {}, {}]", nbr_normal[0], nbr_normal[1], nbr_normal[2]);
      spdlog::info("    T2=[{}, {}, {}]", nbr_tangent[0], nbr_tangent[1], nbr_tangent[2]);
      spdlog::info("    theta {}", theta);
    }
    const auto smoothness = (theta * theta);
    frame_smoothness += smoothness;
  }

  if (should_dump) {
    spdlog::info("Bad {}   Good {} ", bad_count, good_count);
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


void
RoSyOptimiser::vote_for_best_ks(
    const std::shared_ptr<Surfel> &this_surfel,
    const std::shared_ptr<Surfel> &that_surfel,
    const Eigen::Vector3f &tangent,
    std::vector<unsigned int> &shared_frames,
    unsigned short &best_k_ij,
    unsigned short &best_k_ji) const {
  using namespace std;
  using namespace Eigen;

  // Initialise vote
  VoteCounter<pair<unsigned short, unsigned short>> vote_counter{
      // In the event of a tie pick one at random.
      [&](const vector<pair<unsigned short, unsigned short>> &possibilities) -> pair<int, int> {
        uniform_int_distribution <size_t> i(0, possibilities.size() - 1);
        return possibilities[i(m_random_engine)];
      }
  };

  // For each frame that they both appear in
  for (auto frame_index: shared_frames) {
    // Compute best k_ij/k_ji and tally vote
    unsigned short k_ij, k_ji;
    Vector3f nbr_tan_in_surfel_space;
    Vector3f nbr_norm_in_surfel_space;
    this_surfel->transform_surfel_via_frame(
        that_surfel,
        frame_index,
        nbr_norm_in_surfel_space,
        nbr_tan_in_surfel_space);
    best_rosy_vector_pair(
        tangent,
        Vector3f::UnitY(),
        k_ij,
        nbr_tan_in_surfel_space,
        nbr_norm_in_surfel_space,
        k_ji);
    vote_counter.cast_vote({k_ij, k_ji});
  }
  auto winner = vote_counter.winner();

  best_k_ij = winner.first;
  best_k_ji = winner.second;
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &this_node) {
  using namespace std;
  using namespace Eigen;

  const auto &this_surfel = this_node->data();
  const auto &starting_tangent = this_surfel->tangent();
  Vector3f new_tangent{starting_tangent};

  bool should_dump = false;//(this_surfel->id() == "s_32");
  float wsum = 0.0f;
  // For each neighbour of this Surfel
  auto neighbours = m_surfel_graph->neighbours(this_node);
  for (const auto &nbr: neighbours) {
    const auto &nbr_surfel = nbr->data();

    // Get the set of common frames
    auto shared_frames = get_common_frames(this_surfel, nbr_surfel);

    // Optionally generate a set of Ks for all frames
    unsigned short k_ij = 0;
    unsigned short k_ji = 0;
    if (m_vote_for_best_k) {
      vote_for_best_ks(
          this_surfel, nbr_surfel, new_tangent,
          shared_frames, k_ij, k_ji);

      // START DEBUG
      if (should_dump) {
        spdlog::info("Voted for edge to {} ", nbr_surfel->id());
        spdlog::info("  k_ij = {},   k_ji = {}", k_ij, k_ji);
      }
      //END DEBUG
    }

    // For each frame that the two surfels are neighbours in:
    for (unsigned int frame_index: shared_frames) {
      Vector3f v1,t1,n1;
      this_surfel->get_vertex_tangent_normal_for_frame(frame_index, v1, t1, n1);
      t1 = this_surfel->frame_data()[frame_index].transform * new_tangent;

      Vector3f v2, t2, n2;
      nbr_surfel->get_vertex_tangent_normal_for_frame(frame_index, v2, t2, n2);
      float this_surfel_weight = 1.0f;
      float nbr_surfel_weight = 1.0f;
      if (m_weight_for_error) {
        adjust_weights_based_on_error(
            this_surfel,
            nbr_surfel,
            this_surfel_weight,
            nbr_surfel_weight);
      }

      // Compute the best RoSy pair (or voted pair)
      std::pair<Vector3f, Vector3f> best_pair;
      if (m_vote_for_best_k) {
        best_pair = {
            vector_by_rotating_around_n(t1, n1, k_ij),
            vector_by_rotating_around_n(t2, n2, k_ji)
        };
      } else {
        best_pair = best_rosy_vector_pair(
            t1, n1, k_ij,
            t2, n2, k_ji);
        // START DEBUG
        if (should_dump) {
          spdlog::info("Computed for edge to {} ", nbr_surfel->id());
          spdlog::info("  k_ij = {},   k_ji = {}", k_ij, k_ji);
        }
        //END DEBUG
      }
      wsum += this_surfel_weight;
      Vector3f v = (best_pair.first * wsum) + (best_pair.second * nbr_surfel_weight);
      v = this_surfel->frame_data()[frame_index].transform.inverse() * v;
      new_tangent = project_vector_to_plane(v, Vector3f::UnitY()); // Normalizes
    } // Next frame
    set_k(m_surfel_graph, this_node, k_ij, nbr, k_ji);
    if (should_dump) {
      spdlog::info("  Storing edge s_32->{} ", nbr->data()->id());
      spdlog::info("    computed from tan ({}, {}, {})", new_tangent[0], new_tangent[1], new_tangent[2]);
      spdlog::info("    k_ij = {},   k_ji = {}", k_ij, k_ji);
    }
  } // Next neighbour

  // Handle damping
  if (m_damping_factor > 0) {
    new_tangent = (m_damping_factor * starting_tangent) + ((1.0f - m_damping_factor) * new_tangent);
    new_tangent = project_vector_to_plane(new_tangent, Vector3f::UnitY(), true);
  }
  this_node->data()->setTangent(new_tangent);
  this_node->data()->set_rosy_correction(degrees_angle_between_vectors(starting_tangent, new_tangent));
}

void RoSyOptimiser::store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const {
  node->data()->set_rosy_smoothness(smoothness);
}