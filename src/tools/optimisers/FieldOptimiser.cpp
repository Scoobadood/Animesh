//
// Created by Dave Durbin on 8/4/2022.
//

#include "FieldOptimiser.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <Geom/Geom.h>
#include <RoSy/RoSy.h>

FieldOptimiser::FieldOptimiser(int target_iterations) //
    : m_graph{nullptr} //
    , m_state{UNINITIALISED} //
    , m_num_iterations{0} //
    , m_target_iterations{target_iterations} //
    , m_current_level{0} //
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
  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("optimise_posy()");

  auto &graph = (*m_graph)[m_current_level];
  spdlog::info("  PoSy pass {}", m_num_iterations + 1);
  for (const auto &this_node: graph->nodes()) {
  }
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
FieldOptimiser::optimise_rosy() {
  auto trace_log = spdlog::get("optimiser");
  trace_log->trace("optimise_rosy()");

  auto & graph = (*m_graph)[m_current_level];

    spdlog::info("  RoSy pass {}", m_num_iterations + 1 );
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
FieldOptimiser::optimise_once() {
  bool optimisation_complete = false;

  switch (m_state) {
    case UNINITIALISED:throw std::logic_error("Can't optimise when no graph is set");
    case INITIALISED:optimise_begin();
      break;
    case OPTIMISING_ROSY:
      optimise_rosy();
      ++m_num_iterations;
      if( m_num_iterations == m_target_iterations) {
        m_num_iterations = 0;
        m_state = OPTIMISING_POSY;
      }
      break;

    case OPTIMISING_POSY:
      optimise_posy();
      ++m_num_iterations;
      if( m_num_iterations == m_target_iterations) {
        m_state = OPTIMISING_POSY;
        optimisation_complete = true;
      }
      break;
  }
  return optimisation_complete;
}

void
FieldOptimiser::set_graph(std::shared_ptr<MultiResolutionSurfelGraph> graph) {
  m_graph = graph;
  m_state = INITIALISED;
}

