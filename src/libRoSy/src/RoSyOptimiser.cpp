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
#include <spdlog/sinks/basic_file_sink.h>
#include <iomanip>

RoSyOptimiser::RoSyOptimiser(const Properties &properties, std::default_random_engine &rng)
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
  m_vote_for_best_k = m_properties.getBooleanProperty("rosy-vote-for-best-k");
  m_fix_bad_edges = m_properties.getBooleanProperty("rosy-fix-bad-edges", false);

  setup_ssa();

  // DEBUG
  try {
    auto logger = spdlog::basic_logger_mt("rosy-optimiser", "logs/rosy-optimiser-trace.txt");
    logger->set_pattern("[rosy optimiser] [%^%l%$] %v");
  }
  catch (const spdlog::spdlog_ex &ex) {
    std::cout << "Log init failed: " << ex.what() << std::endl;
  }
}

/**
 * @return The total smoothness for a given node in a frame.
 * Also sets the number of neighbours it's compared to so that the mean can be computed.
 */
float
RoSyOptimiser::compute_smoothness_in_frame(
    const SurfelGraph::Edge &edge,
    unsigned int frame_idx) const {

  using namespace Eigen;

  const auto &this_surfel = edge.from()->data();
  Vector3f vertex, normal, tangent;
  this_surfel->get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);

  const auto &nbr_surfel = edge.to()->data();
  Vector3f nbr_vertex, nbr_normal, nbr_tangent;
  nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_tangent, nbr_normal);

  // Compute best Ks
  unsigned short k_ij;
  unsigned short k_ji;
  auto best_pair = best_rosy_vector_pair(tangent, normal, k_ij, nbr_tangent, nbr_normal, k_ji);
  float theta = degrees_angle_between_vectors(best_pair.first, best_pair.second);
  const auto smoothness = (theta * theta);
  return smoothness;
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
RoSyOptimiser::get_weights(const std::shared_ptr<Surfel> &surfel_a,
                           const std::shared_ptr<Surfel> &surfel_b,
                           float &weight_a,
                           float &weight_b) const {
  weight_a = 1.0f;
  weight_b = 1.0f;
  if (m_weight_for_error) {
    adjust_weights_based_on_error(surfel_a, surfel_b, weight_a, weight_b);
  }
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &node) {
  using namespace std;
  using namespace Eigen;

  auto rosy_logger = spdlog::get("rosy-optimiser");

  std::shared_ptr<Surfel> surfel = node->data();
  rosy_logger->info("Optimising surfel {}", surfel->id());

  const auto starting_tangent = surfel->tangent();
  rosy_logger->info("  Starting tangent ({:3f} {:3f} {:3f})",
                    starting_tangent[0],
                    starting_tangent[1],
                    starting_tangent[2]);

  Vector3f new_tangent;

  float weight_sum = 0.0f;

  // For each frame in which this node exists
  for (auto current_frame_idx: surfel->frames()) {

    // Get surfel stats
    Vector3f position, normal;
    surfel->get_vertex_tangent_normal_for_frame(current_frame_idx, position, new_tangent, normal);
    auto transform = surfel->transform_for_frame(current_frame_idx);
    rosy_logger->info("  transform = [{:3f} {:3f} {:3f}; {:3f} {:3f} {:3f}; {:3f} {:3f} {:3f};]",
                      transform(0, 0), transform(0, 1), transform(0, 2),//
                      transform(1, 0), transform(1, 1), transform(1, 2),//
                      transform(2, 0), transform(2, 1), transform(2, 2));

    rosy_logger->info("  tangent_in_frame = [{:3f} {:3f} {:3f}]",
                      new_tangent[0], new_tangent[1], new_tangent[2]);


    // Get the neighbours of this node in the current frame
    auto
        neighbours = get_node_neighbours_in_frame(m_surfel_graph, node, current_frame_idx);

    // Smooth with each neighbour in turn
    for (const auto &nbr: neighbours) {
      const auto &nbr_surfel = nbr->data();
      Vector3f nbr_position, nbr_tangent, nbr_normal;
      nbr_surfel->get_vertex_tangent_normal_for_frame(current_frame_idx,
                                                      nbr_position,
                                                      nbr_tangent,
                                                      nbr_normal);

      rosy_logger->info("  smoothing new_tangent ({:3f} {:3f} {:3f}) with {} ({:3f} {:3f} {:3f}) in frame {}",
                        new_tangent[0], new_tangent[1], new_tangent[2], //
                        nbr_surfel->id(), //
                        nbr_tangent[0], nbr_tangent[1], nbr_tangent[2], //
                        current_frame_idx);

      float this_weight, nbr_weight;
      get_weights(surfel, nbr_surfel, this_weight, nbr_weight);

      // Compute the best RoSy pair
      unsigned short k_ij, k_ji;
      auto best_pair = best_rosy_vector_pair(
          new_tangent, normal, k_ij,
          nbr_tangent, nbr_normal, k_ji);

      rosy_logger->info("    best pair ({:3f} {:3f} {:3f}), ({:3f} {:3f} {:3f}) [{}, {}]",
                        best_pair.first[0], best_pair.first[1], best_pair.first[2], //
                        best_pair.second[0], best_pair.second[1], best_pair.second[2], //
                        k_ij, k_ji);

      new_tangent = (best_pair.first * weight_sum + best_pair.second * nbr_weight);
      weight_sum += this_weight;
      new_tangent = project_vector_to_plane(new_tangent, normal); // Normalizes

      rosy_logger->info("    new_tangent -> ({:3f} {:3f} {:3f})",
                        new_tangent[0], new_tangent[1], new_tangent[2] //
      );

    } // Next neighbour

    // Push back into default space
    new_tangent = transform.inverse() * new_tangent;
    new_tangent = project_vector_to_plane(new_tangent, Vector3f::UnitY()); // Normalizes
    surfel->setTangent(new_tangent);

    rosy_logger->info("  Tangent at end of frame {} tangent ({:3f} {:3f} {:3f})",
                      current_frame_idx,
                      new_tangent[0],
                      new_tangent[1],
                      new_tangent[2]);
  } // Next frame

  rosy_logger->info("  Ending tangent ({:3f} {:3f} {:3f})",
                    new_tangent[0],
                    new_tangent[1],
                    new_tangent[2]);

  auto corrn = degrees_angle_between_vectors(starting_tangent, new_tangent);
  rosy_logger->info("  Correction {:3f}", corrn);

  surfel->set_rosy_correction(corrn);
}

void RoSyOptimiser::store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const {
  node->data()->set_rosy_smoothness(smoothness);
}

void
RoSyOptimiser::label_edge(SurfelGraph::Edge &edge) {
  // Frames in which the edge occurs are frames which feature both start and end nodes
  auto frames_for_edge = get_common_frames(edge.from()->data(), edge.to()->data());

  unsigned short k_ij, k_ji;
  compute_label_for_edge(edge, frames_for_edge, k_ij, k_ji);
  set_k(m_surfel_graph, edge.from(), k_ij, edge.to(), k_ji);
}

void
RoSyOptimiser::compute_label_for_edge( //
    const SurfelGraph::Edge &edge, //
    const std::vector<unsigned int> &frames_for_edge, //
    unsigned short &k_ij, //
    unsigned short &k_ji) const //
{
  using namespace Eigen;

  if (frames_for_edge.size() == 1) {
    Vector3f ignored, t_i, t_j, n_i, n_j;
    edge.from()->data()->get_vertex_tangent_normal_for_frame(frames_for_edge[0], ignored, t_i, n_i);
    edge.to()->data()->get_vertex_tangent_normal_for_frame(frames_for_edge[0], ignored, t_j, n_j);
    best_rosy_vector_pair(t_i, n_i, k_ij, t_j, n_j, k_ji);
    return;
  }

  throw std::runtime_error("multi-frame labeling not supported");
}

/*
 * Apply labels to the edge to indicate the relative rotational frames of
 * the cross field at each end. This is a single integer from 0 .. 3
 * In a single frame these are guaranteed to be consistent. When there are multiple frames,
 * it's possible that different frames have different preferences. In this case we must
 * pick the 'best' orientation. More details on what best means is below.
 *
 */
void
RoSyOptimiser::label_edges() {
  using namespace std;
  using namespace spdlog;

  auto rosy_logger = spdlog::get("rosy-optimiser");
  rosy_logger->trace(">> label_edges()");

  for (auto &edge: m_surfel_graph->edges()) {
    label_edge(edge);
  }

  rosy_logger->trace("<< label_edges()");
}

void RoSyOptimiser::ended_optimisation() {
  label_edges();
}
