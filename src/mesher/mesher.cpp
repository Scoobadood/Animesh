#include "surfel_compute.h"
#include "surfel_io.h"
#include "optimise.h"
#include "mesher_args.h"
#include "surfel_hierarchy.h"
#include "depth_map_io.h"
#include "correspondences_io.h"
#include "correspondences_compute.h"

#include <DepthMap/DepthMapPyramid.h>

void
load_or_compute_correspondences(const MesherArguments& args, std::vector<std::vector<PixelInFrame>>& correspondences ) {
    if( args.load_correspondences_from_file) {
        load_correspondences_from_file(args.correspondence_file_name, correspondences);
    } else {
        compute_correspondences(args.file_or_directory, correspondences);
    }
}

int main(int argc, char *argv[]) {
    using namespace std;

    MesherArguments args;
    parse_args(argc, argv, args);

    vector<DepthMapPyramid> depth_map_pyramids;
    load_depth_map_pyramids(args, depth_map_pyramids);

    vector<vector<PixelInFrame>> correspondences;
    load_or_compute_correspondences(args, correspondences);

    // Construct a hierarchy of surfels
    SurfelHierarchy sh{depth_map_pyramids, correspondences, 0.01f};

    sh.optimise();

    return 0;
}

