#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <Properties/Properties.h>
#include <DepthMap/DepthMap.h>
#include <omp.h>
#include "correspondences_io.h"
#include "surfel_compute.h"
#include "optimise.h"
#include "types.h"
#include "surfel_io.h"
#include "utilities.h"
#include "depth_map_io.h"

const char pre_smooth_filename_template[] = "presmooth_%02d.bin";
const char post_smooth_filename_template[] = "smoothed_%02d.bin";
const char dm_template[] = "depth_map_genned_L%02d_F%02d.pgm";
const char norm_template[] = "normal_map_genned_L%02d_F%02d.ppm";

void
dump_pifs_in_surfel(const std::string &message, const std::vector<Surfel> &surfels) {
    using namespace std;
    cout << message << " surfels" << endl;
    for (const auto &surfel : surfels) {
        cout << "Surfel id : " << surfel.id << " present in " << surfel.frame_data.size() << " frames ";
        for (const auto &frame : surfel.frame_data) {
            cout << frame.pixel_in_frame.frame << ", ";
        }
        cout << endl << "\t";
        for (const auto &frame : surfel.frame_data) {
            cout << "[ f:" << frame.pixel_in_frame.frame << "  x:" << frame.pixel_in_frame.pixel.x << "  y:"
                 << frame.pixel_in_frame.pixel.y << "]";
        }
        cout << endl;
    }
}

std::map<PixelInFrame, size_t>
map_pifs_to_surfel_reference(std::vector<Surfel> &surfels) {
    using namespace std;

    map<PixelInFrame, size_t> pif_to_surfel;
    for (Surfel &surfel : surfels) {
        for (const auto &frame : surfel.frame_data) {
            pif_to_surfel.insert(
                    pair<PixelInFrame, int>(
                            PixelInFrame{frame.pixel_in_frame.pixel.x, frame.pixel_in_frame.pixel.y,
                                         frame.pixel_in_frame.frame},
                            surfel.id));
        }
    }
    return pif_to_surfel;
}

namespace std {
    template<>
    struct std::less<std::reference_wrapper<Surfel>> {
        bool operator()(const reference_wrapper<Surfel> &k1, const reference_wrapper<Surfel> &k2) const {
            return k1.get().id < k2.get().id;
        }
    };
}

std::multimap<size_t, size_t>
compute_surfel_parent_child_mapping(std::vector<Surfel> &parent_level, //
                                    std::vector<Surfel> &child_level, //
                                    std::vector<int> &unmapped) {
    using namespace std;

    // Dump the previous level surfels into a human readable format.
    dump_pifs_in_surfel("Parent level", parent_level);
    // Dump the current level surfels into human readable form.
    dump_pifs_in_surfel("Child level", child_level);

    // Construct a map from parent level PIF to Surfel
    map<PixelInFrame, size_t> pif_to_surfel = map_pifs_to_surfel_reference(parent_level);

    // For each PIF in each surfel in the child level, find the matching PIF and Surfel(s) in upper level
    // Map from child id to parent ids
    multimap<size_t, size_t> surfel_surfel_map;
    for (Surfel &surfel : child_level) {
        int mapping_count = 0;
        for (const auto &frame : surfel.frame_data) {
            PixelInFrame parent_pif{frame.pixel_in_frame.pixel.x / 2,
                                    frame.pixel_in_frame.pixel.y / 2,
                                    frame.pixel_in_frame.frame};
            auto it = pif_to_surfel.find(parent_pif);
            if (it != pif_to_surfel.end()) {
                surfel_surfel_map.insert(pair<size_t, size_t>(surfel.id, parent_level.at(it->second).id));
            }
            ++mapping_count;
        }
        if (mapping_count == 0) {
            unmapped.push_back(surfel.id);
        }
    }
    return surfel_surfel_map;
}

/**
 * Given a set of existing surfels (previous level), populate the current level
 * by propagating the existing level surfel to all 'parent' surfels of the next level.
 * <pre>
   For each Surfel s in previous_level
     For each pixel in frame pif for s
       Derive the candidate source pixels in surfels, pif_next[] (there will be at most four)
       Store reference from pif_next[] to tangent of s

    For each surfel s in surfels
      For each pixel in frame pif for s
        Look up tangent(s) for pif and average them to bootstrap tangent for s

 * @param surfels Vector of surfels to be initialised
 * @param previous_level Vector of surfels from which to initialise
 */
void
down_propagate_tangents(std::multimap<size_t, size_t> &child_to_parents, std::vector<Surfel> &children,
                        std::vector<Surfel> &parents) {

    using namespace std;
    using namespace Eigen;
    cout << "Initialising surfels from previous level" << endl;

    // for each surfel in the lower level
    for (auto child_iterator = child_to_parents.begin(); child_iterator != child_to_parents.end(); child_iterator++) {
        size_t child_id = child_iterator->first;
        int count = 0;
        Vector3f mean_tangent{0.0f, 0.0f, 0.0};
        auto parent_iterator = child_iterator;
        for (; parent_iterator != child_to_parents.end() && (parent_iterator->first == child_id); ++parent_iterator) {
            mean_tangent += parents.at(parent_iterator->second).tangent;
            ++count;
        }
        children.at(child_iterator->first).tangent = (mean_tangent / count).normalized();
        child_iterator = parent_iterator;
    }
}

std::vector<std::vector<PixelInFrame>>
get_correspondences(const Properties &properties,
                    unsigned int level,
                    const std::vector<DepthMap> &depth_map,
                    std::vector<Camera> &cameras) {
    using namespace std;

    vector<vector<PixelInFrame>> correspondences;

    if (properties.getBooleanProperty("load-correspondences")) {
        string corr_file_template = properties.getProperty("correspondence-file-template");
        size_t size = snprintf(nullptr, 0, corr_file_template.c_str(), level) + 1; // Extra space for '\0'
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, corr_file_template.c_str(), level);
        string file_name = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside

        load_correspondences_from_file(file_name, correspondences);
    } else {
        cout << "Computing correspondences from scratch" << endl;
        throw runtime_error("Not implemented. Copy code from dm_to_pointcloud");
    }

    if (correspondences.empty()) {
        throw runtime_error("No correspondences found");
    }
    return correspondences;
}


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

    // +-----------------------------------------------------------------------------------------------
    // | Load properties
    // +-----------------------------------------------------------------------------------------------
    string property_file_name = (argc == 2) ? argv[1] : "animesh.properties";
    Properties properties{property_file_name};

    // +-----------------------------------------------------------------------------------------------
    // | Load depth maps
    // +-----------------------------------------------------------------------------------------------
    vector<DepthMap> depth_maps = load_depth_maps(properties);
    size_t num_frames = depth_maps.size();

    // +-----------------------------------------------------------------------------------------------
    // | Load cameras
    // +-----------------------------------------------------------------------------------------------
    vector<Camera> cameras = load_cameras(num_frames);

    // +-----------------------------------------------------------------------------------------------
    // | Construct the depth map hierarchy: number of levels as specified in properties.
    // +-----------------------------------------------------------------------------------------------
    vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps);
    int num_levels = depth_map_hierarchy.size();
    // Compute normals
    for (unsigned int l = 0; l < num_levels; ++l) {
        for (unsigned int f = 0; f < num_frames; ++f) {
            depth_map_hierarchy.at(l).at(f).compute_normals(cameras.at(f));
        }
    }
    if (properties.getBooleanProperty("dump-depth-maps")) {
        cout << " Dumping depth maps" << endl;
        for (unsigned int level = 0; level < num_levels; ++level) {
            for (unsigned int frame = 0; frame < num_frames; ++frame) {
                auto dm_file_name = file_name_from_template_level_and_frame(dm_template, level, frame);
                save_depth_map_as_pgm(dm_file_name, depth_map_hierarchy.at(level).at(frame));
                auto norm_file_name = file_name_from_template_level_and_frame(norm_template, level, frame);
                save_normals_as_ppm(norm_file_name, depth_map_hierarchy.at(level).at(frame));
            }
        }
    }

    // +-----------------------------------------------------------------------------------------------
    // | Construct Surfels for each level
    // +-----------------------------------------------------------------------------------------------
    size_t surfels_per_step = properties.getIntProperty("surfels-per-step");
    float convergence_threshold = properties.getFloatProperty("convergence-threshold");
    int current_level_index = num_levels - 1;
    vector<Surfel> previous_level;
    while (current_level_index >= 0) {
        cout << "Generating surfels for level : " << current_level_index << endl;

        // +-----------------------------------------------------------------------------------------------
        // | Generate or load correspondences
        // +-----------------------------------------------------------------------------------------------
        // TODO: Seed correspondences for next level Propagate changes down
        cout << " Getting correspondences" << endl;
        vector<vector<PixelInFrame>> correspondences = get_correspondences(properties, current_level_index,
                                                                           depth_map_hierarchy.at(current_level_index),
                                                                           cameras);

        // +-----------------------------------------------------------------------------------------------
        // | Generate Surfels for this level from correspondences
        // +-----------------------------------------------------------------------------------------------
        vector<Surfel> surfels = generate_surfels(depth_map_hierarchy.at(current_level_index), correspondences);

        if (!previous_level.empty()) {
            vector<int> unmapped;
            multimap<size_t, size_t> child_to_parent = compute_surfel_parent_child_mapping(
                    previous_level,
                    surfels,
                    unmapped);

            // Remove unmapped surfels from this level
            if( !unmapped.empty()) {
                vector<Surfel> kept_surfels;
                for( const auto& surfel : surfels) {
                    if( find(begin(unmapped), end(unmapped), surfel.id) != end(unmapped)) {
                        cout << "Dropping surfel with id " << surfel.id << " because parent can't be found" << endl;
                    } else {
                        kept_surfels.push_back( surfel);
                    }
                }
                surfels = kept_surfels;
            }

            // Propagate tangents down
            down_propagate_tangents(child_to_parent, surfels, previous_level);
        }

        // +-----------------------------------------------------------------------------------------------
        // | Save the pre-smoothing surfels in a renderable way
        // +-----------------------------------------------------------------------------------------------
        save_surfels_to_file(file_name_from_template_and_level(pre_smooth_filename_template,
                                                               current_level_index), surfels);

        // +-----------------------------------------------------------------------------------------------
        // | Smooth this level
        // +-----------------------------------------------------------------------------------------------
        Optimiser o{convergence_threshold, num_frames, surfels_per_step};
        o.optimise(surfels);

        // +-----------------------------------------------------------------------------------------------
        // | Save the smoothed surfels in a renderable way
        // +-----------------------------------------------------------------------------------------------
        save_surfels_to_file(file_name_from_template_and_level(post_smooth_filename_template,
                                                               current_level_index), surfels);

        previous_level = surfels;
        --current_level_index;
    }

    return 0;
}
