
#include <PoSy/PoSyOptimiser.h>
#include <Properties/Properties.h>
#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>

#include "spdlog/cfg/env.h"
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>


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


    srandom(123);
    spdlog::cfg::load_env_levels();

    info("Loading properties");
    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    Properties properties{property_file_name};

    string input_file_name = properties.getProperty("posy-input-file");
    string output_file_name = properties.getProperty("posy-output-file");

    PoSyOptimiser poSyOptimiser{properties};
    bool read_smoothness = properties.getBooleanProperty("posy-file-read-smoothness");
    auto surfel_graph = load_surfel_graph_from_file(input_file_name, read_smoothness);
    poSyOptimiser.set_data(surfel_graph);

    auto start_time = std::chrono::system_clock::now();
    unsigned int last_level_iterations = 0;
    auto last_level_start_time = std::chrono::system_clock::now();
    while ((!poSyOptimiser.optimise_do_one_step())) {
        ++last_level_iterations;
    }

    auto end_time = std::chrono::system_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    auto last_level_elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(
            end_time - last_level_start_time).count();

    auto mins = (int) elapsed_time / 60;
    auto secs = elapsed_time - (mins * 60);
    info("Total time {}s ({02d}:{02d})", elapsed_time, mins, secs);
    info("Total iterations : {}", last_level_iterations);

    save_surfel_graph_to_file(output_file_name, surfel_graph);
    info("Saved to {}", output_file_name);

    return 0;
}
