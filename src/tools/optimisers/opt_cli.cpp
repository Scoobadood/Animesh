//
// Created by Dave Durbin on 8/4/2022.
//

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <Properties/Properties.h>
#include <Surfel/MultiResolutionSurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include "FieldOptimiser.h"

std::shared_ptr<MultiResolutionSurfelGraph>
load_graph(const std::string &file_name, int num_levels) {
  if (num_levels <= 0) {
    throw std::runtime_error("Must be at least one level");
  }
  std::default_random_engine rng{123};
  auto surfel_graph = load_surfel_graph_from_file(file_name, rng);
  auto multi_res = std::make_shared<MultiResolutionSurfelGraph>(surfel_graph, rng);
  if (num_levels > 1) {
    multi_res->generate_levels(num_levels);
  }
  return multi_res;
}

void
check_properties(const std::shared_ptr<Properties>& properties) {
  if (properties == nullptr) {
    throw std::invalid_argument("Properties cannot be null");
  }
  if (!properties->hasProperty("input-file")) {
    throw std::invalid_argument("Missing property 'input-file'");
  }
  if (!properties->hasProperty("output-file")) {
    throw std::invalid_argument("Missing property 'output-file'");
  }
  if (!properties->hasProperty("num-iterations")) {
    throw std::invalid_argument("Missing property 'num-iterations'");
  }
  if(properties->hasProperty("rho")) {
    if( properties->getFloatProperty("rho") <= 0.0f) {
      throw std::invalid_argument("Bad value for 'rho'");
    }
  }
}

std::shared_ptr<Properties>
load_properties(const std::string &properties_file_name) {
  spdlog::info("Loading properties from {}", properties_file_name);
  auto properties = std::make_shared<Properties>(properties_file_name);
  check_properties(properties);
  return properties;
}

void
report_timing(const std::chrono::time_point<std::chrono::system_clock> &start_time, //
              const std::chrono::time_point<std::chrono::system_clock> &end_time //
) {
  using namespace std::chrono;
  auto elapsed_time = duration_cast<seconds>(end_time - start_time).count();
  auto mins = (int) elapsed_time / 60;
  auto secs = elapsed_time - (mins * 60);
  spdlog::info("Total time {}s ({:02d}:{:02d})", elapsed_time, mins, secs);
}

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace spdlog;

  cfg::load_env_levels();

  auto properties = load_properties((argc == 2) ? argv[1] : "animesh.properties");
  string input_file_name = properties->getProperty("input-file");

  auto num_levels = properties->hasProperty("num-levels")
                    ? properties->getIntProperty("num-levels")
                    : 1;
  auto graph = load_graph(input_file_name, num_levels);

  auto num_iterations = properties->getIntProperty("num-iterations");
  auto rho = (properties->hasProperty("rho"))
      ? properties->getFloatProperty("rho")
      : 1.0f;

  auto optimiser = std::make_shared<FieldOptimiser>(num_iterations, rho);
  optimiser->set_graph(graph);

  auto start_time = chrono::system_clock::now();
  while (!optimiser->optimise_once());
  auto end_time = chrono::system_clock::now();
  report_timing(start_time, end_time);

  string output_file_name = properties->getProperty("output-file");
  save_surfel_graph_to_file(output_file_name, (*graph)[0], true, true);

  return 0;

}
