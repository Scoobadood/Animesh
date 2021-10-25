
#include <Properties/Properties.h>
#include <Surfel/Surfel_IO.h>
#include <RoSy/MultiResolutionRoSyOptimiser.h>
#include <RoSy/RoSyOptimiser.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include <string>
#include <chrono>

/**
 * Entry point
 * Generate a vector of Surfels and save to disk.
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
  using namespace std;
  using namespace spdlog;

  spdlog::cfg::load_env_levels();

  info("Loading properties");
  string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
  Properties properties{property_file_name};

  string input_file_name = properties.getProperty("rosy-input-file");
  string output_file_name = properties.getProperty("rosy-output-file");

  std::mt19937 rng{123};         // the Mersenne Twister with a popular choice of parameters

  auto surfel_graph = load_surfel_graph_from_file(input_file_name, rng);

  RoSyOptimiser *roSyOptimiser;
  if (properties.hasProperty("enable-multi-resolution") &&
      properties.getBooleanProperty("enable-multi-resolution")) {
    roSyOptimiser = new MultiResolutionRoSyOptimiser(properties, rng);
  } else {
    roSyOptimiser = new RoSyOptimiser(properties, rng);
  }
  roSyOptimiser->set_data(surfel_graph);

  auto start_time = std::chrono::system_clock::now();
  unsigned int iterations = 0;

  while ((!roSyOptimiser->optimise_do_one_step())) {
    ++iterations;
  }
  auto end_time = std::chrono::system_clock::now();
  auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();

  auto mins = (int) elapsed_time / 60;
  auto secs = elapsed_time - (mins * 60);
  info("Total time {}s ({:02d}:{:02d})", elapsed_time, mins, secs);
  info("Iterations : {}", iterations);

  save_surfel_graph_to_file(output_file_name, surfel_graph, true, true);
  info("Saved to {}", output_file_name);

  return 0;
}
