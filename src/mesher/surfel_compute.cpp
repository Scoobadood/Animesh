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

#include "correspondences_compute.h"

/*
	********************************************************************************
	**																			  **
	**					Utilities        										  **
	**																			  **
	********************************************************************************
*/
bool compare_frame_data_by_frame(const FrameData &fd1, const FrameData &fd2) {
    return fd1.pixel_in_frame.frame < fd2.pixel_in_frame.frame;
}

/**
 * Sort all framedata for each surfel in ascending order of frame id.
 * We do this once to facilitate finding common frames.
 */
void
sort_frame_data(std::vector<Surfel> &surfels) {
    for (auto surfel : surfels) {
        sort(surfel.frame_data.begin(), surfel.frame_data.end(), compare_frame_data_by_frame);
    }
}


/*
	********************************************************************************
	**																			  **
	**					Build           										  **
	**																			  **
	********************************************************************************
*/


float
random_zero_to_one() {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1
    return dis(e);
}

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
 * way. If they have the same coordinates in the frame they aree NOT neighbours.
 * @param pif1 The first PixelinFrame.
 * @param pif2 The second PixelinFrame.
 */
bool
are_neighbours(const PixelInFrame &pif1, const PixelInFrame &pif2) {
    if (pif1.frame != pif2.frame) {
        return false;
    }
    int dx = (int) pif1.pixel.x - (int) pif2.pixel.x;
    int dy = (int) pif1.pixel.y - (int) pif2.pixel.y;
    if (dx == 0 && dy == 0) {
        return false;
    }
    if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {
        return true;
    }
    return false;
}

/**
 * Return true if surfel1 and surfel2 are neighbours.
 * S1 is a neighbour of S2 iff:
 * S1 is represented in a frame F by point P1 AND
 * S2 is represented in frame F by point P2 AND
 * P1 and P2 are adjacent in an 8-connected way
 * // TODO: Consider depth disparities. We may have eliminated this as a problem during depth map cleanup but perhaps not.
 * @param surfel1 The first surfel to consider.
 * @param surfel2 The second surfel to consider.
 */
bool
are_neighbours(const Surfel &surfel1, const Surfel &surfel2) {
    using namespace std;

    // Sort framedata for each surfel by frame index
    // TODO: This is an expensive shallow copy operation. We should probably avoid it.
    vector<FrameData> fd1 = surfel1.frame_data;
    sort(fd1.begin(), fd1.end(), compare_frame_data_by_frame);
    vector<FrameData> fd2 = surfel2.frame_data;
    sort(fd2.begin(), fd2.end(), compare_frame_data_by_frame);

    // While both lists have a frame left
    //   if frame at front of both lists are same
    //     if points in that frame are neighbours,
    //       return true
    //     else
    //       discard both frames
    //   else
    //     discard lower frame
    //   endif
    // endwhile
    // return false
    auto it1 = fd1.begin();
    auto it2 = fd2.begin();
    while ((it1 != fd1.end()) && (it2 != fd2.end())) {
        PixelInFrame pif1 = it1->pixel_in_frame;
        PixelInFrame pif2 = it2->pixel_in_frame;
        if (pif1.frame == pif2.frame) {
            if (are_neighbours(pif1, pif2)) {
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
 * The list of neighbours for a surfel is unique, that is, no mater how many frames
 * contain projections of S and Sn which are neighbours, Sn will occur only once in 
 * the ilst of S's neighbours.
 * @param surfels The list of all surfels.
 * @param neighbours 
 */
void
populate_neighbours(std::vector<Surfel> &surfels) {
    using namespace std;

    cout << "Populating neighbour : " << flush;
    if( surfels.empty() ) {
        throw runtime_error(" No surfels found in populate_neighbours");
    }
    int target = surfels.size();

    for (int i = 0, count = 0; i < surfels.size() - 1; ++i) {
        cout << "\rPopulating neighbour : " << ++count << " of " << target << flush;
        for (int j = i + 1; j < surfels.size(); ++j) {
            if (are_neighbours(surfels.at(i), surfels.at(j))) {
                surfels.at(i).neighbouring_surfels.push_back(j);
                surfels.at(j).neighbouring_surfels.push_back(i);
            }
        }
    }
    cout << endl;
}

/**
 * For each entry in the correspondence group, we need to construct a 
 * FrameData object which tells us the frame and pixel affected as
 * well as the required transform.
 */
void
populate_frame_data(const std::vector<PixelInFrame> &correspondence_group,
                    const std::vector<DepthMap> &depth_maps,
                    std::vector<FrameData> &frame_data) {
    using namespace std;
    using namespace Eigen;

    Vector3f y{0.0, 1.0, 0.0};
    for (auto c : correspondence_group) {
        vector<float> n = depth_maps.at(c.frame).get_normals().at(c.pixel.y).at(c.pixel.x);
        Vector3f target_normal{n.at(0), n.at(1), n.at(2)};
        float depth = depth_maps.at(c.frame).depth_at(c.pixel.y, c.pixel.x);
        FrameData fd{c, depth, vector_to_vector_rotation(y, target_normal), target_normal};
        frame_data.push_back(fd);
    }
}


/*
	********************************************************************************
	**																			  **
	**                                 Top Level Calls                            **
	**																			  **
    ********************************************************************************
*/

/*
	For each correspondence
		Make a surfel
		-- For each Frame/Pixel mapping from that correspondence compute the one we will use
		-- Compute the normal (or get it)
		-- Cmpute the transformation matrix
		-- Store the Frame Data
		-- Init the random direction vector.
 */
std::vector<Surfel>
generate_surfels(const std::vector<DepthMap> &depth_maps,
                 const std::vector<std::vector<PixelInFrame>> &correspondences) {
    using namespace std;
    assert(correspondences.size() > 0);

    cout << "Generating surfels : " << endl;

    vector<Surfel> surfels;

    // One per correpondence -- except that a correspondence that references a dead/remove pixel should be ignored
    // For each one, add point, depth, normal
    map<PixelInFrame, size_t> pixel_in_frame_to_surfel;

    int count = 0;
    int target = correspondences.size();
    for (auto const &corresponding_pifs : correspondences) {
        Surfel surfel;

        cout << "Considering possible surfel : " << ++count << " of possible " << target << endl;
        cout << "   Correspondence group: ";
        for (auto pif : corresponding_pifs) {
            cout << "   { f:" << pif.frame << "  x:" << pif.pixel.x << "  y:" << pif.pixel.y <<"} ";
        }
        cout << endl;


        // Although the pixels in frames may be in correspondence, it's not necessarily true that
        // each of them has a valid normal (it may have insufficient neighbours, be on an edge or
        // whatever.
        vector<PixelInFrame> pifs_with_normals;
        for (auto pif : corresponding_pifs) {
            // Force normal generation
            depth_maps[pif.frame].is_normal_defined(pif.pixel.y, pif.pixel.x);
            auto nn = depth_maps[pif.frame].normal_at(pif.pixel.x, pif.pixel.y);
            cout << "  cg: { f:" << pif.frame << "  x:" << pif.pixel.x << "  y:" << pif.pixel.y <<"} --> Norm: {"<<nn.type << " " << nn.x << " " << nn.y << " " << nn.z << "}" << endl;

            if (depth_maps[pif.frame].is_normal_defined(pif.pixel.y, pif.pixel.x)) {
                pifs_with_normals.push_back(pif);
            }
        }
        if (!pifs_with_normals.empty()) {
            surfel.id = surfels.size();
            populate_frame_data(pifs_with_normals, depth_maps, surfel.frame_data);
            surfels.push_back(surfel);
            for (auto pif : pifs_with_normals) {
                pixel_in_frame_to_surfel.insert(make_pair<>(pif, surfel.id));
            }
            cout << "\t surfel generated" << endl;
        } else {
            cout << "\t rejected - no normals defined for any of  " << corresponding_pifs.size() << " pifs " << endl;
        }
    }
    cout << endl << " generated " << surfels.size() << " surfels" << endl;
    populate_neighbours(surfels);
    randomize_tangents(surfels);
}

/*
	********************************************************************************
	**																			  **
	**                                 Entry Point                                **
	**																			  **
	********************************************************************************
*/
