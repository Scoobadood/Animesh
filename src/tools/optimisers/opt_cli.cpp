//
// Created by Dave Durbin on 8/4/2022.
//

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <Properties/Properties.h>
#include <Surfel/MultiResolutionSurfelGraph.h>
#include <Surfel/Surfel_IO.h>

int main(int argc, char *argv[]) {
  using namespace std;
  using namespace spdlog;

  cfg::load_env_levels();

  string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
  info("Loading properties from {}", property_file_name);
  Properties properties{property_file_name};

  string input_file_name = properties.getProperty("input-file");
  string output_file_name = properties.getProperty("output-file");

  std::default_random_engine rng{123};
  auto surfel_graph = load_surfel_graph_from_file(input_file_name, rng);
  auto multi_res = std::make_shared<MultiResolutionSurfelGraph>(surfel_graph, rng);
}
