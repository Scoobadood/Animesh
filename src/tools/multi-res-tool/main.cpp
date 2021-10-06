//
// Created by Dave Durbin (Old) on 26/9/21.
//

#include <tclap/CmdLine.h>
#include <tclap/ArgException.h>
#include <spdlog/spdlog.h>
#include <Surfel/Surfel_IO.h>
#include <Surfel/MultiResolutionSurfelGraph.h>

struct args {
  std::string input_graph_file_name;
  unsigned int levels;
};

args parse_args(int argc, char *argv[]) {
  using namespace TCLAP;
  using namespace std;

  try {
    CmdLine cmd("Convert graph to multi-res", ' ', "0.1");

    // Define a value argument and add it to the command line.
    // A value arg defines a flag and a type of value that it expects,
    // such as "-n Bishop".
    ValueArg <string> input_file("g", "input_graph_file_name", "The input graph file name", true, "", "file name", cmd);
    ValueArg<int> levels("l", "levels", "How many levels to generate.", false, 3, "int", cmd);

    // Parse the argv array.
    cmd.parse(argc, argv);

    args a{};
    a.input_graph_file_name = input_file.getValue();
    a.levels = levels.getValue();

    return a;
  } catch (TCLAP::ArgException &e) { // catch any exceptions
    spdlog::error("error: {} for argument {}.", e.error(), e.argId());
    throw std::runtime_error{"Bad arguments"};
  }
}


int main(int argc, char *argv[]) {
  using namespace std;

  auto args = parse_args(argc, argv);
  spdlog::info("Reading from {} and generating {} levels.",
               args.input_graph_file_name,
               args.levels
  );

  // Normalise and centre
  auto surfel_graph = load_surfel_graph_from_file(args.input_graph_file_name);
  spdlog::info("Loaded {} nodes", surfel_graph->num_nodes());

  MultiResolutionSurfelGraph mrg{surfel_graph};
  mrg.generate_levels(args.levels);

  spdlog::info("Writing {} levels", args.levels);

  for( int i=1; i<args.levels; ++i) {
    save_surfel_graph_to_file(args.input_graph_file_name + "_" + to_string(i), mrg[i]);
  }
}