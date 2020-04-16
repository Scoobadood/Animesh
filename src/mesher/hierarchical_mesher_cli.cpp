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

    while( ! o.optimise_do_one_step()) {
        info("Done a step");
    }

    return 0;
}
