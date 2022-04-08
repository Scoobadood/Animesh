//
// Created by Dave Durbin on 8/4/2022.
//

#include "FieldOptimiser.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <Geom/Geom.h>
#include <RoSy/RoSy.h>
#include <PoSy/PoSy.h>

FieldOptimiser::FieldOptimiser( //
    int target_iterations //
    , float rho //
) //
    : m_graph{nullptr} //
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
  m_num_iterations = 0;
  m_state = OPTIMISING_ROSY;
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
  for (const auto &node: graph->nodes()) {

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

        // Compute the point that minimizes the distance to vertices while being located in their respective tangent plane
        const auto midpoint = compute_qij(curr_surfel_pos, curr_surfel_normal, nbr_surfel_pos, nbr_surfel_normal);
        trace_log->info("        midpoint=[{:.3f} {:.3f} {:.3f}];", midpoint[0], midpoint[1], midpoint[2]);

        // The midpoint is contained on both tangent planes. Find the lattice points on both planes
        // which are closest to each other - checks 4 points surrounding midpoint on both planes
        // Origin, reptan, normal, point
        auto curr_surfel_base =
            position_floor(working_clp, curr_surfel_tangent, curr_surfel_normal, midpoint, m_rho);
        auto nbr_surfel_base =
            position_floor(nbr_surfel_clp, nbr_surfel_tangent, nbr_surfel_normal, midpoint, m_rho);

        auto best_cost = std::numeric_limits<float>::infinity();
        int best_i = -1, best_j = -1;
        for (int i = 0; i < 4; ++i) {
          // Derive ,o0t (test)  sequentiall, the other bounds of 1_ij in first plane
          Vector3f o0t =
              curr_surfel_base + (curr_surfel_tangent * (i & 1) + curr_surfel_orth_tangent * ((i & 2) >> 1)) * m_rho;
          for (int j = 0; j < 4; ++j) {
            Vector3f o1t =
                nbr_surfel_base
                    + (nbr_surfel_tangent * (j & 1) + nbr_surfel_orth_tangent * ((j & 2) >> 1)) * m_rho;
            auto cost = (o0t - o1t).squaredNorm();
            if (cost < best_cost) {
              best_i = i;
              best_j = j;
              best_cost = cost;
            }
          }
        }

        // best_i and best_j are the closest vertices surrounding q_ij
        // we return the pair of closest points on the lattice according to both origins
        auto closest_points = std::make_pair(
            curr_surfel_base
                + (curr_surfel_tangent * (best_i & 1) + curr_surfel_orth_tangent * ((best_i & 2) >> 1)) * m_rho,
            nbr_surfel_base
                + (nbr_surfel_tangent * (best_j & 1) + nbr_surfel_orth_tangent * ((best_j & 2) >> 1)) * m_rho);

        {
          trace_log->info("        curr_closest=[{:.3f} {:.3f} {:.3f}];",
                          closest_points.first[0], closest_points.first[1], closest_points.first[2]);
          trace_log->info("        nbr_closest=[{:.3f} {:.3f} {:.3f}];",
                          closest_points.second[0], closest_points.second[1], closest_points.second[2]);
        }

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
        working_clp = round_4(curr_surfel_normal, curr_surfel_tangent, working_clp, curr_surfel_pos, m_rho);
        trace_log->info("        rounded and normalised working_clp=[{:.3f} {:.3f} {:.3f}];",
                        working_clp[0], working_clp[1], working_clp[2]);

        // Convert the new LP into a
      } // Next neighbour

      // Convert back to offset.
      auto clp_offset = working_clp - curr_surfel_pos;
      auto u = clp_offset.dot(curr_surfel_tangent);
      auto v = clp_offset.dot(curr_surfel_orth_tangent);
      trace_log->info("        new_lattice_offset=[{:.3f} {:.3f}];", u, v);

      node->data()->set_reference_lattice_offset({u, v});
    } // Next frame
  }
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
FieldOptimiser::optimise_rosy() {
  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("optimise_rosy()");

  auto &graph = (*m_graph)[m_current_level];

  spdlog::info("  RoSy pass {}", m_num_iterations + 1);
  for (const auto &this_node: graph->nodes()) {
    using namespace std;
    using namespace Eigen;


    std::shared_ptr<Surfel> this_surfel = this_node->data();
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
      Vector3f v, this_surfel_normal_in_frame;
      this_surfel->get_vertex_tangent_normal_for_frame(current_frame_idx,
                                                       v,
                                                       new_tangent,
                                                       this_surfel_normal_in_frame);
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
        Vector3f nbr_tangent_in_frame, nbr_normal_in_frame;
        nbr_surfel->get_vertex_tangent_normal_for_frame(current_frame_idx, v,
                                                        nbr_tangent_in_frame,
                                                        nbr_normal_in_frame);

        trace_log->info("  smoothing new_tangent ({:3f} {:3f} {:3f}) with {} ({:3f} {:3f} {:3f}) in frame {}",
                        new_tangent[0], new_tangent[1], new_tangent[2], //
                        nbr_surfel->id(), //
                        nbr_tangent_in_frame[0], nbr_tangent_in_frame[1], nbr_tangent_in_frame[2], //
                        current_frame_idx);

        float this_weight, nbr_weight;
        get_weights(this_surfel, nbr_surfel, this_weight, nbr_weight);

        // Compute the best RoSy pair
        pair<Vector3f, Vector3f> best_pair;
        unsigned short k_ij, k_ji;
        best_pair = best_rosy_vector_pair(
            new_tangent, this_surfel_normal_in_frame, k_ij,
            nbr_tangent_in_frame, nbr_normal_in_frame, k_ji);

        trace_log->info("    best pair ({:3f} {:3f} {:3f}), ({:3f} {:3f} {:3f}) [{}, {}]",
                        best_pair.first[0], best_pair.first[1], best_pair.first[2], //
                        best_pair.second[0], best_pair.second[1], best_pair.second[2], //
                        k_ij, k_ji);

        weight_sum += this_weight;
        new_tangent = (best_pair.first * weight_sum + best_pair.second * nbr_weight);
        new_tangent = project_vector_to_plane(new_tangent, this_surfel_normal_in_frame); // Normalizes

        trace_log->info("    new_tangent -> ({:3f} {:3f} {:3f})",
                        new_tangent[0], new_tangent[1], new_tangent[2] //
        );

      } // Next neighbour
      new_tangent = this_surfel_transform * new_tangent;
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
}

bool
FieldOptimiser::propagate_values() {
  using namespace Eigen;

  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("propagate_values()");
  if( m_current_level == 0) {
    return true;
  }

  m_graph->propagate(m_current_level, true, true);
  --m_current_level;
  m_state = OPTIMISING_ROSY;
  return false;
}

bool
FieldOptimiser::optimise_once() {
  bool optimisation_complete = false;

  switch (m_state) {
    case UNINITIALISED:throw std::logic_error("Can't optimise when no graph is set");
    case INITIALISED:optimise_begin();
      break;
    case OPTIMISING_ROSY:optimise_rosy();
      ++m_num_iterations;
      if (m_num_iterations == m_target_iterations) {
        m_num_iterations = 0;
        m_state = OPTIMISING_POSY;
      }
      break;

    case OPTIMISING_POSY:optimise_posy();
      ++m_num_iterations;
      if (m_num_iterations == m_target_iterations) {
        m_num_iterations = 0;
        m_state = PROPAGATE_VALUES;
      }
      break;

    case PROPAGATE_VALUES:optimisation_complete = propagate_values();
      break;
  }
  return optimisation_complete;
}

void
FieldOptimiser::set_graph(std::shared_ptr<MultiResolutionSurfelGraph> graph) {
  m_graph = graph;
  m_state = INITIALISED;
}

