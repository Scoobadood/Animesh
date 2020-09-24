
#include <PoSy/PoSyOptimiser.h>
#include <Properties/Properties.h>
#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>

#include <Utilities/utilities.h>

#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include <string>
#include <vector>
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

    spdlog::cfg::load_env_levels();

    info("Loading properties");
    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    Properties properties{property_file_name};

    PoSyOptimiser poSyOptimiser{properties};
    auto surfel_graph = load_surfel_graph_from_file("smoothed_01.bin");
    poSyOptimiser.set_data(surfel_graph);

    auto start_time = std::chrono::system_clock::now();
    unsigned int last_level_iterations = 0;
    auto last_level_start_time = std::chrono::system_clock::now();
    while( ! poSyOptimiser.optimise_do_one_step()) {
        ++last_level_iterations;
    }

    auto end_time = std::chrono::system_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    auto last_level_elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - last_level_start_time).count();

    auto mins = (int) elapsed_time / 60;
    auto secs = elapsed_time - (mins * 60);
    cout << "Total time " << elapsed_time <<"s  (" << mins << ":" << setw(2) << setfill('0') << secs << ")" << endl;
    cout << "Total iterations : " << ( last_level_iterations) << endl;

    mins = (int) last_level_elapsed_time / 60;
    secs = last_level_elapsed_time - (mins * 60);
    cout << "Last level time " << last_level_elapsed_time <<"s  (" << mins << ":" << setw(2) << setfill('0') << secs << ")" << endl;
    cout << "Last level iterations : " << last_level_iterations << endl;

    save_surfel_graph_to_file("posy_smoothed_01.bin", surfel_graph);
    cout << "Saved" << endl;

    return 0;
}
