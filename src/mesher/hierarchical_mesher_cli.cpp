//#include <memory>
#include "optimise.h"
#include "hierarchical_mesher_utilities.h"
#include <Properties/Properties.h>
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"

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

    Optimiser o{properties};

    info("Loading depth maps");
    vector<DepthMap> depth_maps = load_depth_maps(properties);
    size_t num_frames = depth_maps.size();

    info("Loading cameras");
    vector<Camera> cameras = load_cameras(num_frames);

    o.set_data(depth_maps, cameras);

    auto start_time = std::chrono::system_clock::now();
    unsigned int hierarchy_iterations = 0;
    while( ( o.get_current_level() != 0 ) && (! o.optimise_do_one_step())) {
        ++hierarchy_iterations;
    }

    unsigned int last_level_iterations = 0;
    auto last_level_start_time = std::chrono::system_clock::now();
    while( ! o.optimise_do_one_step()) {
        ++last_level_iterations;
    }

    auto end_time = std::chrono::system_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    auto last_level_elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - last_level_start_time).count();

    auto mins = (int) elapsed_time / 60;
    auto secs = elapsed_time - (mins * 60);
    cout << "Total time " << elapsed_time <<"s  (" << mins << ":" << setw(2) << setfill('0') << secs << ")" << endl;
    cout << "Total iterations : " << (hierarchy_iterations + last_level_iterations) << endl;

    mins = (int) last_level_elapsed_time / 60;
    secs = last_level_elapsed_time - (mins * 60);
    cout << "Last level time " << last_level_elapsed_time <<"s  (" << mins << ":" << setw(2) << setfill('0') << secs << ")" << endl;
    cout << "Last level iterations : " << last_level_iterations << endl;
    cout << "Smoothness : " << o.get_mean_error() << endl;

    return 0;
}
