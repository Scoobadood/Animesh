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
            cout << frame.pixel_in_frame << " ";
        }
        cout << endl;
    }
}

/**
 * Create a map to allow lookup of a Surfel ID from one of its PIF
 */
std::map<PixelInFrame, std::string>
map_pifs_to_surfel_id(const std::vector<Surfel> &surfels) {
    using namespace std;

    map<PixelInFrame, string> pif_to_surfel_id;
    for (const Surfel &surfel : surfels) {
        for (const auto &frame_data : surfel.frame_data) {
            pif_to_surfel_id.emplace(frame_data.pixel_in_frame, surfel.id);
        }
    }
    return pif_to_surfel_id;
}

namespace std {
    template<>
    struct std::less<std::reference_wrapper<Surfel>> {
        bool operator()(const reference_wrapper<Surfel> &k1, const reference_wrapper<Surfel> &k2) const {
            return k1.get().id < k2.get().id;
        }
    };
}

/**
 * Given a set of parent surfels and child surfels, estalish a mapping from child to one or more parents.
 * Children with no parents are stored as IDs in unmapped
 */
std::multimap<std::string, std::string>
compute_child_to_parent_surfel_id_map(std::vector<Surfel> &child_surfels, //
                                      std::vector<Surfel> &parent_surfels, //
                                      std::vector<std::string> &orphans) {
    using namespace std;

    // Dump the previous level surfels into a human readable format.
    dump_pifs_in_surfel("Parent level", parent_surfels);
    // Dump the current level surfels into human readable form.
    dump_pifs_in_surfel("Child level", child_surfels);

    // Construct a map from parent level PIF to Surfel id
    map<PixelInFrame, string> pif_to_parent_id = map_pifs_to_surfel_id(parent_surfels);

    // For each PIF in each child surfel, try to find a matching PIF in the parent surfels map
    multimap<string, string> child_to_parents_surfel_id_map;

    for (const auto &child_surfel : child_surfels) {
        unsigned int parents_found = 0;
        for (const auto &child_frame : child_surfel.frame_data) {
            PixelInFrame parent_pif{child_frame.pixel_in_frame.pixel.x / 2, child_frame.pixel_in_frame.pixel.y / 2,
                                    child_frame.pixel_in_frame.frame};
            const auto &it = pif_to_parent_id.find(parent_pif);
            if (it != pif_to_parent_id.end()) {
                child_to_parents_surfel_id_map.emplace(child_surfel.id, it->second);
                ++parents_found;
            } // else parent not found for this PIF
        }
        if (parents_found == 0) {
            orphans.push_back(child_surfel.id);
        }
    }
    return child_to_parents_surfel_id_map;
}

/**
 * Initialise the child surfel tangents from their psarwent surfel tangents
 * where the parent-child mappings are defined in child_to_parents
 */
void
down_propagate_tangents(const std::multimap<std::string, std::string> &child_to_parents) {

    using namespace std;
    using namespace Eigen;
    cout << "Propagating surfel tangents from previous level" << endl;

    // for each surfel in the lower level
    auto child_iterator = child_to_parents.begin();
    while ( child_iterator != child_to_parents.end()) {
        auto child_id = child_iterator->first;
        int num_parents = 0;
        Vector3f mean_tangent{0.0f, 0.0f, 0.0};
        auto parent_iterator = child_iterator;
        while ((parent_iterator != child_to_parents.end()) && (parent_iterator->first == child_id)) {
            mean_tangent += Surfel::surfel_for_id(parent_iterator->second).tangent;
            ++num_parents;
            ++parent_iterator;
        }
        Surfel::surfel_for_id(child_iterator->first).tangent = (mean_tangent / num_parents).normalized();
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


void
maybe_save_depth_and_normal_maps(const Properties &properties,
                                 const std::vector<std::vector<DepthMap>> &depth_map_hierarchy) {
    using namespace std;

    int num_levels = depth_map_hierarchy.size();
    int num_frames = depth_map_hierarchy.at(0).size();
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
}

/**
 * Remove any surfels which cannot be found from the neighbours of remaining surfels
 */
void
prune_surfel_neighbours(std::vector<Surfel> &surfels) {
    using namespace std;

    // Stash the surfel IDs that exist into a map with their new index
    vector<string> existing_surfel_ids{surfels.size()};
    for (const auto &surfel : surfels) {
        existing_surfel_ids.push_back(surfel.id);
    }
    sort(existing_surfel_ids.begin(), existing_surfel_ids.end());

    // Now update lists of neighbours to remove ones that have gone
    for (auto &surfel : surfels) {
        int old_neighbour_count = surfel.neighbouring_surfels.size();
        cout << "Neighbours of " << surfel.id << " before pruning" << endl;
        for( const auto& id : surfel.neighbouring_surfels ) {
            cout << "\t" << id << endl;
        }
        surfel.neighbouring_surfels.erase(
                remove_if(surfel.neighbouring_surfels.begin(), surfel.neighbouring_surfels.end(),
                          [existing_surfel_ids](const string &neighbour_id) {
                              return binary_search(existing_surfel_ids.begin(), existing_surfel_ids.end(), neighbour_id);
                          }), surfel.neighbouring_surfels.end()
        );
        cout << "Neighbours of " << surfel.id << " after pruning" << endl;
        for( const auto& id : surfel.neighbouring_surfels ) {
            cout << "\t" << id << endl;
        }
        cout << "\t" << (old_neighbour_count - surfel.neighbouring_surfels.size()) << " neighbours erased" << endl;
    }
}

/**
 * Remove the previous level surfels from the Surfel::map
 */
void
unmap_surfels(const std::vector<std::string>& surfel_ids ) {
    using namespace std;

    for( const auto & k : surfel_ids) {
        Surfel::surfel_by_id.erase(k);
    }
}

/**
 * Remove the previous level surfels from the Surfel::map
 */
void
unmap_surfels(const std::vector<Surfel>& surfels ) {
    using namespace std;

    vector<string> ids_to_remove;
    for_each(surfels.begin(),
             surfels.end(),
             [&ids_to_remove](const Surfel &s) { ids_to_remove.push_back(s.id); });
    unmap_surfels(ids_to_remove);
}

/**
 * Given a list of surfel IDs, remove them from the surfel list.
 * The properties object is consulted to check whether to log this removal or not.
 */
void
remove_surfels_by_id(std::vector<Surfel> &surfels, std::vector<std::string> &ids_to_remove, const Properties &properties) {
    using namespace std;

    size_t initial_surfel_count = surfels.size();
    if (!ids_to_remove.empty()) {
        sort(ids_to_remove.begin(), ids_to_remove.end());
        surfels.erase(remove_if(surfels.begin(), surfels.end(), [ids_to_remove](const Surfel &s) {
            return binary_search(ids_to_remove.begin(), ids_to_remove.end(), s.id);
        }), surfels.end());

        // Shrink the surfel vector to erase removed surfels
        unmap_surfels(ids_to_remove);

        // optionally log
        if (properties.getBooleanProperty("log-dropped-surfels")) {
            cout << "Removed " << ids_to_remove.size() << " of " << initial_surfel_count << " surfels. Surfels remaining: " << surfels.size() << endl;
            for( const auto& id : ids_to_remove) {
                cout << "  " << id << endl;
            }
        }
    }
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
    vector<vector<DepthMap>> depth_map_hierarchy = create_depth_map_hierarchy(properties, depth_maps, cameras);
    maybe_save_depth_and_normal_maps(properties, depth_map_hierarchy);
    int num_levels = depth_map_hierarchy.size();

    // +-----------------------------------------------------------------------------------------------
    // | Construct Surfels for each level
    // +-----------------------------------------------------------------------------------------------
    size_t surfels_per_step = properties.getIntProperty("surfels-per-step");
    float convergence_threshold = properties.getFloatProperty("convergence-threshold");
    int current_level_index = num_levels - 1;
    vector<Surfel> previous_level_surfels;
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
        vector<Surfel> current_level_surfels = generate_surfels(depth_map_hierarchy.at(current_level_index), correspondences, properties);

        // +-----------------------------------------------------------------------------------------------
        // | Propagate tangents down
        // +-----------------------------------------------------------------------------------------------
        if (!previous_level_surfels.empty()) {
            vector<string> orphans;
            multimap<string, string> child_to_parent_surfel_id_map = compute_child_to_parent_surfel_id_map(
                    current_level_surfels,
                    previous_level_surfels,
                    orphans);
            // Remove orphan surfels from this level
            remove_surfels_by_id(current_level_surfels, orphans, properties);

            // Seed the current level surfels with tangents from their parents.
            down_propagate_tangents(child_to_parent_surfel_id_map);

            // Prune neighbours to handle removed surfels
            prune_surfel_neighbours(current_level_surfels);

            // Remove previous level surfels
            unmap_surfels(previous_level_surfels);
        }

        // +-----------------------------------------------------------------------------------------------
        // | Save the pre-smoothing surfels in a renderable way
        // +-----------------------------------------------------------------------------------------------
        save_surfels_to_file(file_name_from_template_and_level(pre_smooth_filename_template,
                                                               current_level_index), current_level_surfels);

        // +-----------------------------------------------------------------------------------------------
        // | Smooth this level
        // +-----------------------------------------------------------------------------------------------
        Optimiser o{convergence_threshold, num_frames, surfels_per_step};
        o.optimise(current_level_surfels);

        // +-----------------------------------------------------------------------------------------------
        // | Save the smoothed surfels in a renderable way
        // +-----------------------------------------------------------------------------------------------
        save_surfels_to_file(file_name_from_template_and_level(post_smooth_filename_template,
                                                               current_level_index), current_level_surfels);

        previous_level_surfels = current_level_surfels;
        --current_level_index;
    }

    return 0;
}
