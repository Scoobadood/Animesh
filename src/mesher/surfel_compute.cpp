#ifdef DEBUG
#include <iostream>
#endif

#include <map>
#include <set>
#include <regex>
#include <random>
#include <iostream>
#include "surfel_compute.h"
#include <DepthMap/DepthMap.h>
#include <Geom/geom.h>
#include "spdlog/spdlog.h"

std::map<std::string,std::reference_wrapper<Surfel>> Surfel::surfel_by_id;

/**
 * Construct Surfels from correspondences.
 * Correspondences are provided as a vector of correspondences where each entry is a vector of pixel in frame.
 * These pixels in frame are used to generate a Surfel where a Surfel has a mapping to each frame in which the
 * surfel is projected. The mapping has a 3D rotation matrix specifying how the normal is rotated from (0,1,0) to
 * the specofioc normal for that pixel in frame.
 * The steps we take are:
 *
 * For each correspondence
 *   Make an empty surfel
 *   For each Frame/Pixel mapping from that correspondence
 *     Compute the normal (or get it)
 *     Compute the transformation matrix from Y axis to normal
 *     Store the Frame Data
 *     Init the random tangent direction vector (perp to normal)
 */


/**
 * Generate a random float in the range [0, 1)
 */
float
random_zero_to_one() {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0, 1); // range 0 - 1
    return dis(e);
}

/**
 * Randomize the tangents of the vec topr of Surfels.
 * Tangents will be set to a radius of the unit circle in the xy plane
 */
void
randomize_tangents(std::vector<Surfel> &surfels) {
    for (auto &surfel : surfels) {
        float xc = random_zero_to_one();
        float yc = sqrt(1.0f - (xc * xc));
        surfel.tangent = Eigen::Vector3f{xc, 0.0f, yc};
    }
}

/**
 * Return true if the two pixel in frames are neighbours.
 * They are neighbours if they are in the same frame and adjacent in an 8-connected
 * way. If they have the same coordinates in the frame they are NOT neighbours.
 * @param pif1 The first PixelinFrame.
 * @param pif2 The second PixelinFrame.
 */
bool
are_neighbours(const PixelInFrame &pif1, const PixelInFrame &pif2, bool use_eight_connected) {
    if (pif1.frame != pif2.frame) {
        return false;
    }
    int dx = (int) pif1.pixel.x - (int) pif2.pixel.x;
    int dy = (int) pif1.pixel.y - (int) pif2.pixel.y;
    if (dx == 0 && dy == 0) {
        return false;
    }

    return use_eight_connected
           ? (std::abs(dx) <= 1 && std::abs(dy) <= 1)
           : ((std::abs(dx) + std::abs(dy)) == 1);
}

/**
 * Return true if surfel1 and surfel2 are neighbours.
 * S1 is a neighbour of S2 iff:
 * S1 is represented in a frame F by pixel_in_frame P1 AND
 * S2 is represented in frame F by pixel_in_frame P2 AND
 * P1 and P2 are adjacent
 * @param surfel1 The first surfel to consider.
 * @param surfel2 The second surfel to consider.
 */
bool
are_neighbours(const Surfel &surfel1, const Surfel &surfel2, bool eight_connected) {
    using namespace std;

    // Sort framedata for each surfel by frame index
    // TODO: This is an expensive shallow copy operation. We should probably avoid it.
    vector<FrameData> fd1 = surfel1.frame_data;
    sort(fd1.begin(), fd1.end());
    vector<FrameData> fd2 = surfel2.frame_data;
    sort(fd2.begin(), fd2.end());

    auto it1 = fd1.begin();
    auto it2 = fd2.begin();
    while ((it1 != fd1.end()) && (it2 != fd2.end())) {
        PixelInFrame& pif1 = it1->pixel_in_frame;
        PixelInFrame& pif2 = it2->pixel_in_frame;
        if (pif1.frame == pif2.frame) {
            if (are_neighbours(pif1, pif2, eight_connected)) {
                return true;
            } else {
                ++it1;
                ++it2;
            }
        } else if (pif1.frame < pif2.frame) {
            ++it1;
        } else {
            ++it2;
        }
    }
    return false;
}

/**
 * For a particular surfel, populate the list of neighbouring surfels.
 * A surfel Sn is a neighbour of another surfel S iff:
 * S is represented in a frame F by point P AND
 * Sn is represented in frame F by point Pn AND
 * P and Pn are neighbours.
 * The list of neighbours for a surfel is unique, that is, no matter how many frames
 * contain projections of S and Sn which are neighbours, Sn will occur only once in 
 * the list of S's neighbours.
 * @param surfels The list of all surfels.
 * @param neighbours 
 */
void
populate_neighbours(std::vector<Surfel> &surfels, bool eight_connected) {
    using namespace std;
    using namespace spdlog;

//    cout << "Populating neighbour : " << flush;
    assert(!surfels.empty());

    for (unsigned int i = 0; i < surfels.size() - 1; ++i) {
        Surfel& surfel = surfels.at(i);
        info ("Populating neighbours of surfel : {:d}", i);
//        cout << "\tAlready found " << surfel.neighbouring_surfels.size() << " neighbours" << endl;
        for (unsigned int j = i + 1; j < surfels.size(); ++j) {
            if (are_neighbours(surfel, surfels.at(j), eight_connected)) {
//                cout << "\tFound new neighbour : " << surfels.at(j).id << endl;
                surfel.neighbouring_surfels.push_back(surfels.at(j).id);
                surfels.at(j).neighbouring_surfels.push_back(surfel.id);
            }
        }
//        cout << "\tFinal count is " << surfel.neighbouring_surfels.size() << " neighbours" << endl;
    }
    for( auto & s : surfels) {
        sort(s.neighbouring_surfels.begin(), s.neighbouring_surfels.end());
    }
}

/**
 * For each entry in the correspondence group, we need to construct a 
 * FrameData object which tells us the frame and pixel affected as
 * well as the required transform.
 */
std::vector<FrameData>
populate_frame_data(const std::vector<PixelInFrame> &pifs_in_correspondence_group,
                    const std::vector<DepthMap> &depth_maps) {
    using namespace std;
    using namespace Eigen;

    Vector3f y_axis{0.0, 1.0, 0.0};
    vector<FrameData> frame_data;
    for (const auto& pif : pifs_in_correspondence_group) {
        const auto& normal = depth_maps.at(pif.frame).normal_at(pif.pixel.x, pif.pixel.y);
        Vector3f target_normal{normal.x, normal.y, normal.z};
        float depth = depth_maps.at(pif.frame).depth_at(pif.pixel.x, pif.pixel.y);

        frame_data.emplace_back(pif, depth, vector_to_vector_rotation(y_axis, target_normal), target_normal);
    }
    return frame_data;
}

/**
 * Given a vector of pixel-in-frame items and depth maps,
 * remove the pifs that have no corresponding normal in the depth map.
 * @param corresponding_pifs Source vector of PIFs
 * @param depth_maps Depth maps for checking normals
 * @return Filtered list of PIFs
 */
std::vector<PixelInFrame>
filter_pifs_with_normals(const std::vector<PixelInFrame> &corresponding_pifs,
                         const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    // Although the pixels in frames may be in correspondence, it's not necessarily true that
    // each of them has a valid normal (it may have insufficient neighbours, be on an edge or
    // whatever. Filter those out.
    vector<PixelInFrame> pifs_with_normals;
    for (const auto &pif : corresponding_pifs) {
        if (depth_maps.at(pif.frame).is_normal_defined(pif.pixel.x, pif.pixel.y)) {
            pifs_with_normals.push_back(pif);

            const auto & nn = depth_maps.at(pif.frame).normal_at(pif.pixel.x, pif.pixel.y);
            cout << pif << " --> Normal: {"
                 << nn.type << " " << nn.x << " " << nn.y << " " << nn.z << "}" << endl;
        } else {
            cout << "  skipping " << pif << " --> No normal" << endl;
        }
    }
    return pifs_with_normals;
}

std::string
generate_uuid() {
    using namespace std;

    static random_device dev;
    static mt19937 rng(dev());

    uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { false, false, false, true, false, false, false, true, false, false, false, true, false, false, false };

    string res;
    for (bool i : dash) {
        if (i) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

/**
 * Actually build a Surfel from the source data.
 */
Surfel
generate_surfel(const std::vector<PixelInFrame> &corresponding_pifs,
                const std::vector<DepthMap> &depth_maps) {
    using namespace std;

    assert( !corresponding_pifs.empty());

    return Surfel{
        generate_uuid(),
        populate_frame_data(corresponding_pifs, depth_maps),
        vector<string>{},
        Eigen::Vector3f::Zero()};
}



/* ******************************************************************************** *
 * * Tested to this point                                                         * *
 * ******************************************************************************** */


/**
 * Given depth maps and correspondences, compute a vector of surfels
 */
std::vector<Surfel>
generate_surfels(const std::vector<DepthMap> &depth_maps,
                 const std::vector<std::vector<PixelInFrame>> &correspondences,
                 const Properties& properties) {
    using namespace std;
    assert(!correspondences.empty());
    assert(!depth_maps.empty());

    bool eight_connected = properties.getBooleanProperty("eight-connected");

    vector<Surfel> surfels;

    int count = 0;
    int target = correspondences.size();
    // Iterate over each correspondence and generate a surfel.
    for (auto const &corresponding_pifs : correspondences) {
        cout << "Considering possible surfel : " << ++count << " of " << target << " candidates" << endl;

        // Although the pixels in frames may be in correspondence, it's not necessarily true that
        // each of them has a valid normal (it may have insufficient neighbours, be on an edge or
        // whatever. Filter them.
        const auto pifs_with_normals = filter_pifs_with_normals(corresponding_pifs, depth_maps);
        if( pifs_with_normals.empty()) {
            cout << "\t rejected - no normals defined for any of  " << corresponding_pifs.size() << " pifs " << endl;
            continue;
        }

        Surfel surfel = generate_surfel(pifs_with_normals, depth_maps);
        surfels.push_back(surfel);
    }
    for( auto& s : surfels) {
        Surfel::surfel_by_id.emplace(s.id, s);
    }

    // Build inter-surfel neighbour list
    populate_neighbours(surfels, eight_connected);

    // Initialise tangents to random, legal values.
    randomize_tangents(surfels);
    cout << endl << " generated " << surfels.size() << " surfels" << endl;
    return surfels;
}