//
// Created by Dave Durbin on 8/4/2022.
//

#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <Properties/Properties.h>
#include <Surfel/MultiResolutionSurfelGraph.h>
#include <Surfel/Surfel_IO.h>

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
check_properties(const std::shared_ptr<Properties> properties) {
  if (properties == nullptr) {
    throw std::invalid_argument("Properties cannot be null");
  }
  if (!properties->hasProperty("input-file")) {
    throw std::invalid_argument("Missing property 'input-file')");
  }
}

std::shared_ptr<Properties>
load_properties(const std::string &properties_file_name) {
  spdlog::info("Loading properties from {}", properties_file_name);
  auto properties = std::make_shared<Properties>(properties_file_name);
  check_properties(properties);
  return properties;
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

  
}
