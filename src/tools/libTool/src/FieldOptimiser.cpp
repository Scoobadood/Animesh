//
// Created by Dave Durbin on 8/4/2022.
//

#include "FieldOptimiser.h"
#include <Eigen/Geometry>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <Geom/Geom.h>
#include <RoSy/RoSy.h>
#include <PoSy/PoSy.h>

FieldOptimiser::FieldOptimiser( //
    std::default_random_engine &rng,
    int target_iterations //
    , float rho //
) //
    : m_random_engine{rng} //
    , m_graph{nullptr} //
    , m_state{UNINITIALISED} //
    , m_num_iterations{0} //
    , m_target_iterations{target_iterations} //
    , m_current_level{0} //
    , m_rho{rho} //
{
  try {
    auto logger = spdlog::basic_logger_mt("optimiser", "logs/optimiser-trace.txt");
    logger->set_pattern("[rosy optimiser] [%^%l%$] %v");
  }
  catch (const spdlog::spdlog_ex &ex) {
    std::cout << "Log init failed: " << ex.what() << std::endl;
  }

}

void
FieldOptimiser::get_weights(const std::shared_ptr<Surfel> &surfel_a,
                            const std::shared_ptr<Surfel> &surfel_b,
                            float &weight_a,
                            float &weight_b) const {
  weight_a = 1.0f;
  weight_b = 1.0f;
}

void
FieldOptimiser::optimise_begin() {
  spdlog::get("optimiser")->trace("optimise_begin()");
  assert(m_state == INITIALISED);

  m_current_level = m_graph->num_levels() - 1;
  m_state = STARTING_NEW_LEVEL;
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
FieldOptimiser::optimise_posy() {
  using namespace Eigen;

  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("optimise_posy()");

  auto &graph = (*m_graph)[m_current_level];
  spdlog::info("  PoSy pass {}", m_num_iterations + 1);

  const auto &nodes = graph->nodes();
  auto indices = randomise_indices(nodes.size());

  for (auto node_index: indices) {
    auto node = nodes[node_index];

    const auto &curr_surfel = node->data();
    trace_log->info("Optimising surfel {}", curr_surfel->id());

    // Ref latt offset in the default space.
    Vector2f curr_lattice_offset = curr_surfel->reference_lattice_offset();
    trace_log->info("  starting lattice offset (default) is ({:.3f}, {:.3f})",
                    curr_lattice_offset[0], curr_lattice_offset[1]);

    // For each frame
    trace_log->info("  surfel is present in {} frames", curr_surfel->frames().size());
    for (const auto frame_idx: curr_surfel->frames()) {
      trace_log->info("  smoothing frame {}", frame_idx);

      // Compute the lattice offset 3D position in the given frame
      Vector3f curr_surfel_pos, curr_surfel_tangent, curr_surfel_normal;
      curr_surfel->get_vertex_tangent_normal_for_frame(frame_idx,
                                                       curr_surfel_pos,
                                                       curr_surfel_tangent,
                                                       curr_surfel_normal);
      const auto curr_surfel_orth_tangent = curr_surfel_normal.cross(curr_surfel_tangent);
      Vector3f working_clp = curr_surfel_pos +
          curr_lattice_offset[0] * curr_surfel_tangent +
          curr_lattice_offset[1] * curr_surfel_orth_tangent;
      trace_log->info("    CLP starts at ({:3f} {:3f} {:3f})", working_clp[0], working_clp[1], working_clp[2]);

      // Get the neighbours of this surfel in this frame
      const auto &neighbours = get_node_neighbours_in_frame(graph, node, frame_idx);
      trace_log->info("    smoothing with {} neighbours", neighbours.size());

      float sum_w = 0.0;
      for (const auto &nbr_node: neighbours) {
        const auto &nbr_surfel = nbr_node->data();
        const auto &nbr_lattice_offset = nbr_surfel->reference_lattice_offset();

        // Compute the neighbour's lattice offset 3D position in the given frame
        Vector3f nbr_surfel_pos, nbr_surfel_tangent, nbr_surfel_normal;
        nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx,
                                                        nbr_surfel_pos,
                                                        nbr_surfel_tangent,
                                                        nbr_surfel_normal);
        const auto nbr_surfel_orth_tangent = nbr_surfel_normal.cross(nbr_surfel_tangent);
        Vector3f nbr_surfel_clp = nbr_surfel_pos +
            nbr_lattice_offset[0] * nbr_surfel_tangent +
            nbr_lattice_offset[1] * nbr_surfel_orth_tangent;
        trace_log->info("      Neighbour {} at ({:.3f} {:.3f} {:.3f}) thinks CLP is at ({:.3f} {:.3f} {:.3f})",
                        nbr_surfel->id(),
                        nbr_surfel_pos[0],
                        nbr_surfel_pos[1],
                        nbr_surfel_pos[2],
                        nbr_surfel_clp[0],
                        nbr_surfel_clp[1],
                        nbr_surfel_clp[2]);

        auto closest_points = compute_closest_lattice_points(
            curr_surfel_pos,
            curr_surfel_normal,
            curr_surfel_tangent,
            curr_surfel_orth_tangent,
            working_clp,
            nbr_surfel_pos,
            nbr_surfel_normal,
            nbr_surfel_tangent,
            nbr_surfel_orth_tangent,
            nbr_surfel_clp,
            m_rho);

        // Compute the weighted mean closest point
        float w_j = 1.0f;
        working_clp = ((sum_w * closest_points.first) + (w_j * closest_points.second));
        sum_w += w_j;
        working_clp /= sum_w;

        trace_log->info("        updated working_clp=[{:.3f} {:.3f} {:.3f}];",
                        working_clp[0], working_clp[1], working_clp[2]);

        // new_lattice_vertex is not necessarily on the plane of the from tangents
        // We may need to correct for this later
        working_clp -= curr_surfel_normal.dot(working_clp - curr_surfel_pos) * curr_surfel_normal;

        // new_lattice_vertex is now a point that we'd like to assume is on the lattice.
        // If this defines the lattice, now find the closest lattice point to curr_surfel_pos
        working_clp =
            position_round(working_clp, curr_surfel_tangent, curr_surfel_orth_tangent, curr_surfel_pos, m_rho);
        trace_log->info("        rounded and normalised working_clp=[{:.3f} {:.3f} {:.3f}];",
                        working_clp[0], working_clp[1], working_clp[2]);
      } // Next neighbour

      // Convert back to offset.
      auto clp_offset = working_clp - curr_surfel_pos;
      auto u = clp_offset.dot(curr_surfel_tangent);
      auto v = clp_offset.dot(curr_surfel_orth_tangent);
      trace_log->info("        new_lattice_offset=[{:.3f} {:.3f}];", u, v);

      node->data()->set_reference_lattice_offset({u, v});
    } // Next frame
  }

  ++m_num_iterations;
  if (m_num_iterations == m_target_iterations) {
    m_state = ENDING_LEVEL;
  }
}

std::vector<size_t>
FieldOptimiser::randomise_indices(unsigned long number) {
  using namespace std;
  vector<size_t> indices(number);
  iota(begin(indices), end(indices), 0);
  shuffle(begin(indices), end(indices), m_random_engine);
  return indices;
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
FieldOptimiser::optimise_rosy() {
  using namespace std;
  using namespace Eigen;

  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("optimise_rosy()");

  auto &graph = (*m_graph)[m_current_level];

  spdlog::info("  RoSy pass {}", m_num_iterations + 1);

  const auto &nodes = graph->nodes();
  auto indices = randomise_indices(nodes.size());

  for (auto node_index: indices) {
    auto this_node = nodes[node_index];
    shared_ptr<Surfel> this_surfel = this_node->data();
    trace_log->info("Optimising surfel {}", this_surfel->id());

    const auto starting_tangent = this_surfel->tangent();
    trace_log->info("  Starting tangent ({:3f} {:3f} {:3f})",
                    starting_tangent[0],
                    starting_tangent[1],
                    starting_tangent[2]);

    Vector3f new_tangent;

    float weight_sum = 0.0f;

    // For each frame in which this node exists
    for (auto current_frame_idx: this_surfel->frames()) {
      // Get my transformation matrix
      Vector3f this_surfel_position, this_surfel_normal;
      this_surfel->get_vertex_tangent_normal_for_frame(current_frame_idx,
                                                       this_surfel_position,
                                                       new_tangent,
                                                       this_surfel_normal);
      auto this_surfel_transform = this_surfel->transform_for_frame(current_frame_idx);
      trace_log->info("  transform = [{:3f} {:3f} {:3f}; {:3f} {:3f} {:3f}; {:3f} {:3f} {:3f};]",
                      this_surfel_transform(0, 0), this_surfel_transform(0, 1), this_surfel_transform(0, 2),//
                      this_surfel_transform(1, 0), this_surfel_transform(1, 1), this_surfel_transform(1, 2),//
                      this_surfel_transform(2, 0), this_surfel_transform(2, 1), this_surfel_transform(2, 2));

      trace_log->info("  tangent_in_frame = [{:3f} {:3f} {:3f}]",
                      new_tangent[0], new_tangent[1], new_tangent[2]);

      // Get the neighbours of this node in the current frame
      auto
          neighbours = get_node_neighbours_in_frame(graph, this_node, current_frame_idx);

      // Smooth with each neighbour in turn
      for (const auto &nbr: neighbours) {
        const auto &nbr_surfel = nbr->data();
        Vector3f nbr_position, nbr_tangent, nbr_normal;
        nbr_surfel->get_vertex_tangent_normal_for_frame(current_frame_idx,
                                                        nbr_position,
                                                        nbr_tangent,
                                                        nbr_normal);

        trace_log->info("  smoothing new_tangent ({:3f} {:3f} {:3f}) with {} ({:3f} {:3f} {:3f}) in frame {}",
                        new_tangent[0], new_tangent[1], new_tangent[2], //
                        nbr_surfel->id(), //
                        nbr_tangent[0], nbr_tangent[1], nbr_tangent[2], //
                        current_frame_idx);

        float this_weight, nbr_weight;
        get_weights(this_surfel, nbr_surfel, this_weight, nbr_weight);

        // Compute the best RoSy pair
        pair<Vector3f, Vector3f> best_pair;
        unsigned short k_ij, k_ji;
        best_pair = best_rosy_vector_pair(
            new_tangent, this_surfel_normal, k_ij,
            nbr_tangent, nbr_normal, k_ji);

        trace_log->info("    best pair ({:3f} {:3f} {:3f}), ({:3f} {:3f} {:3f}) [{}, {}]",
                        best_pair.first[0], best_pair.first[1], best_pair.first[2], //
                        best_pair.second[0], best_pair.second[1], best_pair.second[2], //
                        k_ij, k_ji);

        new_tangent = (best_pair.first * weight_sum + best_pair.second * nbr_weight);
        weight_sum += this_weight;
        new_tangent = project_vector_to_plane(new_tangent, this_surfel_normal); // Normalizes

        trace_log->info("    new_tangent -> ({:3f} {:3f} {:3f})",
                        new_tangent[0], new_tangent[1], new_tangent[2] //
        );

      } // Next neighbour
      new_tangent = this_surfel_transform.inverse() * new_tangent;
      new_tangent = project_vector_to_plane(new_tangent, Vector3f::UnitY()); // Normalizes
      this_surfel->setTangent(new_tangent);

      trace_log->info("  Tangent at end of frame {} tangent ({:3f} {:3f} {:3f})",
                      current_frame_idx,
                      new_tangent[0],
                      new_tangent[1],
                      new_tangent[2]);

    } // Next frame

    trace_log->info("  Ending tangent ({:3f} {:3f} {:3f})",
                    new_tangent[0],
                    new_tangent[1],
                    new_tangent[2]);

    auto corrn = degrees_angle_between_vectors(starting_tangent, new_tangent);
    trace_log->info("  Correction {:3f}", corrn);

    this_surfel->set_rosy_correction(corrn);
  }

  ++m_num_iterations;
  if (m_num_iterations == m_target_iterations) {
    m_num_iterations = 0;
    m_state = OPTIMISING_POSY;
  }
}

void
FieldOptimiser::end_level() {
  using namespace Eigen;

  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("end_level()");
  if (m_current_level == 0) {
    m_state = LABEL_EDGES;
    return;
  }

  m_graph->propagate_completely(m_current_level, true, true);
  --m_current_level;
  m_state = STARTING_NEW_LEVEL;
}

void
FieldOptimiser::start_level() {
  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("starting_level()");
  spdlog::info("Starting level {}", m_current_level);
  m_num_iterations = 0;
  m_state = OPTIMISING_ROSY;
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
FieldOptimiser::label_edges() {
  using namespace std;
  using namespace spdlog;

  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("label_edges()");

  for (auto &edge: (*m_graph)[0]->edges()) {
    label_edge(edge);
  }

  m_state = DONE;
}

std::vector<unsigned int>
get_common_frames(
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

void
FieldOptimiser::compute_k_for_edge( //
    const std::shared_ptr<Surfel> &from_surfel,
    const std::shared_ptr<Surfel> &to_surfel,
    unsigned int frame_idx,
    unsigned short &k_ij, //
    unsigned short &k_ji) const //
{
  Eigen::Vector3f ignored, t_i, t_j, n_i, n_j;
  from_surfel->get_vertex_tangent_normal_for_frame(frame_idx, ignored, t_i, n_i);
  to_surfel->get_vertex_tangent_normal_for_frame(frame_idx, ignored, t_j, n_j);
  best_rosy_vector_pair(t_i, n_i, k_ij, t_j, n_j, k_ji);
}

void
FieldOptimiser::compute_t_for_edge( //
    const std::shared_ptr<Surfel> &from_surfel, //
    const std::shared_ptr<Surfel> &to_surfel, //
    unsigned int frame_idx, //
    Eigen::Vector2i &t_ij, //
    Eigen::Vector2i &t_ji) const//
{
  using namespace Eigen;

  // Then compute CLP for both vertices in this frame
  const auto &clp_offset1 = from_surfel->reference_lattice_offset();
  Vector3f v1, t1, n1;
  from_surfel->get_vertex_tangent_normal_for_frame(frame_idx, v1, t1, n1);
  auto clp1 = v1 +
      m_rho * clp_offset1[0] * t1 +
      m_rho * clp_offset1[1] * (n1.cross(t1));

  const auto &clp_offset2 = to_surfel->reference_lattice_offset();
  Vector3f v2, t2, n2;
  to_surfel->get_vertex_tangent_normal_for_frame(frame_idx, v2, t2, n2);
  auto clp2 = v2 +
      m_rho * clp_offset2[0] * t2 +
      m_rho * clp_offset2[1] * (n2.cross(t2));

  auto t = compute_tij_tji(v1, n1, t1, n1.cross(t1), clp1,
                           v2, n2, t2, n2.cross(t2), clp2, m_rho);


  // Then compute t_ij and t_ji
  t_ij = t.first;
  t_ji = t.second;
}

void
FieldOptimiser::label_edge(SurfelGraph::Edge &edge) {
  // Frames in which the edge occurs are frames which feature both start and end nodes
  auto frames_for_edge = get_common_frames(edge.from()->data(), edge.to()->data());
  if (frames_for_edge.size() > 1) {
    throw std::logic_error("No common frames for edge between " +
        edge.from()->data()->id() +
        " and " +
        edge.to()->data()->id());
  }
  if (frames_for_edge.size() > 1) {
    throw std::runtime_error("multi-frame labeling not supported");
  }

  auto from_surfel = edge.from()->data();
  auto to_surfel = edge.to()->data();
  auto frame_idx = frames_for_edge[0];

  unsigned short k_ij, k_ji;
  compute_k_for_edge(from_surfel, to_surfel, frame_idx, k_ij, k_ji);
  set_k((*m_graph)[0], edge.from(), k_ij, edge.to(), k_ji);

  Eigen::Vector2i t_ij, t_ji;
  compute_t_for_edge(from_surfel, to_surfel, frame_idx, t_ij, t_ji);
  set_t((*m_graph)[0], edge.from(), t_ij, edge.to(), t_ji);
}

bool
FieldOptimiser::optimise_once() {
  bool optimisation_complete = false;

  switch (m_state) {
    case UNINITIALISED:throw std::logic_error("Can't optimise when no graph is set");
    case INITIALISED:optimise_begin();
      break;
    case STARTING_NEW_LEVEL:start_level();
      break;

    case OPTIMISING_ROSY:optimise_rosy();
      break;

    case OPTIMISING_POSY:optimise_posy();
      break;

    case ENDING_LEVEL: end_level();
      break;

    case LABEL_EDGES: label_edges();
      break;

    case DONE:optimisation_complete = true;
  }
  return optimisation_complete;
}

void
FieldOptimiser::set_graph(std::shared_ptr<MultiResolutionSurfelGraph> graph) {
  m_graph = graph;
  m_state = INITIALISED;
}

