#ifdef DEBUG
#include <iostream>
#endif

#include <map>
#include <regex>
#include <random>
#include <iostream>
#include <memory>
#include <DepthMap/DepthMap.h>
#include <Geom/Geom.h>
#include <Properties/Properties.h>
#include "Surfel_Compute.h"
#include "PixelInFrame.h"
#include "Surfel.h"
#include "SurfelBuilder.h"
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

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
are_neighbours(const std::shared_ptr<Surfel> &surfel1, const std::shared_ptr<Surfel> &surfel2, bool eight_connected) {
    using namespace std;

    auto it1 = begin(surfel1->frame_data());
    auto it2 = begin(surfel2->frame_data());
    while ((it1 != end(surfel1->frame_data())) && (it2 != end(surfel2->frame_data()))) {
        const PixelInFrame &pif1 = it1->pixel_in_frame;
        const PixelInFrame &pif2 = it2->pixel_in_frame;
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
SurfelGraphPtr
graph_from_surfels(std::vector<std::shared_ptr<Surfel>> &surfels, bool eight_connected) {
    using namespace std;
    using namespace spdlog;

    SurfelGraphPtr graph = std::make_shared<SurfelGraph>(new SurfelGraph());

    assert(!surfels.empty());

    // Add surfels to graph
    map<string, SurfelGraphNodePtr> m;
    for (const auto &surfel : surfels) {
        auto node = graph->add_node(surfel);
        m.emplace(surfel->id(), node);
    }

    for (unsigned int i = 0; i < surfels.size() - 1; ++i) {
        auto &surfel = surfels.at(i);
        debug("Populating neighbours of surfel : {:d}", i);
        for (unsigned int j = i + 1; j < surfels.size(); ++j) {
            if (are_neighbours(surfel, surfels.at(j), eight_connected)) {
                auto node1 = m.at(surfels.at(j)->id());
                auto node2 = m.at(surfels.at(i)->id());

                graph->add_edge(node1, node2, SurfelGraphEdge{1.0});
                graph->add_edge(node2, node1, SurfelGraphEdge{1.0});
            }
        }
    }
    return graph;
}

/**
 * For each entry in the correspondence group, we need to construct a 
 * FrameData object which tells us the frame and pixel affected as
 * well as the required transform.
 */
std::vector<FrameData>
populate_frame_data(const std::vector<PixelInFrame> &pifs_in_correspondence_group,
                    const std::vector<DepthMap> &depth_maps_by_frame,
                    const std::map<PixelInFrame, Eigen::Vector3f> &coordinates_by_pif) {
    using namespace std;
    using namespace Eigen;

    Vector3f y_axis{0.0, 1.0, 0.0};
    vector<FrameData> frame_data;
    for (const auto &pif : pifs_in_correspondence_group) {
        const auto &normal = depth_maps_by_frame.at(pif.frame).normal_at(pif.pixel.x, pif.pixel.y);
        Vector3f target_normal{normal.x, normal.y, normal.z};
        auto depth = depth_maps_by_frame.at(pif.frame).depth_at(pif.pixel.x, pif.pixel.y);
        auto target_position = coordinates_by_pif.at(pif);

        frame_data.emplace_back(pif, depth, vector_to_vector_rotation(y_axis, target_normal), target_normal,
                                target_position);
    }
    std::sort(frame_data.begin(), frame_data.end());
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
    stringstream msg;
    for (const auto &pif : corresponding_pifs) {
        if (depth_maps.at(pif.frame).is_normal_defined(pif.pixel.x, pif.pixel.y)) {
            pifs_with_normals.push_back(pif);

            const auto &nn = depth_maps.at(pif.frame).normal_at(pif.pixel.x, pif.pixel.y);
            msg << pif << " --> Normal: {"
                << nn.type << " " << nn.x << " " << nn.y << " " << nn.z << "}" << endl;
        } else {
            msg << "  skipping " << pif << " --> No normal" << endl;
        }
        spdlog::debug(msg.str());
    }
    return pifs_with_normals;
}


/**
 * Actually build a Surfel from the source data.
 */
std::shared_ptr<Surfel>
generate_surfel(SurfelBuilder *surfel_builder,
                const std::vector<PixelInFrame> &corresponding_pifs,
                const std::vector<DepthMap> &depth_maps_by_frame,
                const std::map<PixelInFrame, Eigen::Vector3f> &coordinates_by_pif) {
    using namespace std;

    assert(!corresponding_pifs.empty());

    auto frame_data = populate_frame_data(corresponding_pifs, depth_maps_by_frame, coordinates_by_pif);
    for (const auto &fd : frame_data) {
        surfel_builder->with_frame(fd);
    }
    auto surfel = make_shared<Surfel>(surfel_builder->build());
    return surfel;
}



/* ******************************************************************************** *
 * * Tested to this point                                                         * *
 * ******************************************************************************** */


/**
 * Given depth maps and correspondences, compute a vector of surfels
 */
SurfelGraphPtr
generate_surfels(const std::vector<DepthMap> &depth_maps,
                 const std::vector<std::vector<PixelInFrame>> &correspondences,
                 const std::map<PixelInFrame, Eigen::Vector3f> &coordinates_by_pif,
                 const Properties &properties) {
    using namespace std;
    assert(!correspondences.empty());
    assert(!depth_maps.empty());

    vector<shared_ptr<Surfel>> surfels;

    std::default_random_engine random_engine{123};
    auto surfel_builder = new SurfelBuilder(random_engine);
    int count = 0;
    int target = correspondences.size();
    // Iterate over each correspondence and generate a surfel.
    for (auto const &corresponding_pifs : correspondences) {
        spdlog::debug("Considering possible surfel : {:d} of {:d} candidates", ++count, target);

        // Although the pixels in frames may be in correspondence, it's not necessarily true that
        // each of them has a valid normal (it may have insufficient neighbours, be on an edge or
        // whatever. Filter them.
        const auto pifs_with_normals = filter_pifs_with_normals(corresponding_pifs, depth_maps);
        if (pifs_with_normals.empty()) {
            spdlog::debug("\t rejected - no normals defined for any of {:d} pifs", corresponding_pifs.size());
            continue;
        }

        auto surfel = generate_surfel(surfel_builder, pifs_with_normals, depth_maps, coordinates_by_pif);
        surfels.push_back(surfel);
    }

    // Sort neighbours and frames
    for (auto &s : surfels) {
        Surfel::m_surfel_by_id.emplace(s->id(), s);
    }

    auto surfel_graph = graph_from_surfels(surfels, properties.getBooleanProperty("eight-connected"));

    spdlog::info(" generated {:d} surfels", surfels.size());
    return surfel_graph;
}