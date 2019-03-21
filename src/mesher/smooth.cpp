#include "smooth.h"
#include <Eigen/Core>
#include <random>
#include <math.h>
#include <RoSy/RoSy.h>

static bool g_is_optimising = false;
static float g_optimisation_error;

/*
   ********************************************************************************
   **                                                                            **
   **             Graph Related                                                  **
   **                                                                            **
   ********************************************************************************
*/

SurfelGraphPtr
make_surfel_graph(const std::vector<Surfel>& surfels) {
	using namespace std;

    SurfelGraphPtr graph = make_shared<SurfelGraph>();
	
	for( size_t idx = 0; idx < surfels.size(); ++idx ) {
		graph->add_node(idx);
	}

	// Connect with edges
    vector<SurfelGraphNode *> nodes = graph->nodes( );
	size_t idx = 0;
	for( auto surfel : surfels ) {
		// Lookup my neighbour indices
		for( auto neighbour_idx : surfel.neighbouring_surfels ) {
			graph->add_edge(nodes.at(idx), nodes.at(neighbour_idx), 1.0f, nullptr);
		}
		idx++;
	}
	return graph;
}

/*
   ********************************************************************************
   **                                                                            **
   **             Utilities                                                      **
   **                                                                            **
   ********************************************************************************
*/

float 
random_zero_to_one( ) {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0, 1); // rage 0 - 1
    return dis(e);
}

std::size_t 
random_index( unsigned int max_index ) {
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(0, max_index);
    return std::floor(dis(e));
}

/*
   ********************************************************************************
   **                                                                            **
   **             Optimisation                                                   **
   **                                                                            **
   ********************************************************************************
*/

void 
randomize_tangents(std::vector<Surfel>& surfels) {
	for (auto surfel : surfels){
		float xc = random_zero_to_one( );
		float yc = sqrt(1.0f - (xc * xc));
		surfel.tangent = Eigen::Vector3f{xc, 0.0f, yc};
	}
}


void 
optimise_begin(std::vector<Surfel>& surfels) {
	randomize_tangents( surfels );
	g_is_optimising = true;
}

void 
optimise_end() {
	g_is_optimising = false;
}

float 
total_error() {
	std::cout << "total_error() not yet implemented" << std::endl;
	return 0.0f;
}

bool 
check_convergence(float error) {
	std::cout << "check_convergence() not yet implemented" << std::endl;
	return true;
}


bool compareFrameDataByFrame(const FrameData &fd1, const FrameData &fd2) {
    return fd1.frame_idx < fd2.frame_idx;
}


/**
 * Populate tangents and normals with all eligible tans norms from surfles neighbours
 * tan/norm is eligible iff the nieghbour and surfel share a common frame
 * tan/norm are converted to the orignating surfel's frame of reference.
 */
void
get_eligible_normals_and_tangents(	const std::vector<Surfel>& surfels, 
									std::size_t surfel_idx, 
									std::vector<Eigen::Vector3f>& normals,
									std::vector<Eigen::Vector3f>& tangents) {
	using namespace std;

	// Create sorted vectors of frame data for each
	vector<FrameData> surfel_frames = surfels.at(surfel_idx).frame_data;
	sort(surfel_frames.begin(), surfel_frames.end(), compareFrameDataByFrame);

	for ( size_t neighbour_idx : surfels.at(surfel_idx).neighbouring_surfels) {
		unsigned long  max_intersectional_values = min(surfel_frames.size(), surfels.at(surfel_idx).neighbouring_surfels.size());

		vector<FrameData> neighbour_frames = surfels.at(neighbour_idx).frame_data;
		sort(neighbour_frames.begin(), neighbour_frames.end(), compareFrameDataByFrame);

		// Find the intersection of these sets
		auto surfel_frame_iter = surfel_frames.begin();
		auto neighbour_frame_iter = neighbour_frames.begin();

		vector<FrameData> eligible_surfel_frames{max_intersectional_values};
		vector<FrameData> eligible_neighbour_frames{max_intersectional_values};
		auto surfel_result_iter = eligible_surfel_frames.begin();
		auto neighbour_result_iter = eligible_neighbour_frames.begin();

		while (surfel_frame_iter != surfel_frames.end() && neighbour_frame_iter != neighbour_frames.end()) {
    		if (surfel_frame_iter->frame_idx < neighbour_frame_iter->frame_idx) {
    			++surfel_frame_iter;
    		} else if (neighbour_frame_iter->frame_idx < surfel_frame_iter->frame_idx) {
    			++neighbour_frame_iter;
    		} else {
      			*surfel_result_iter = *surfel_frame_iter;
      			*neighbour_result_iter = *neighbour_frame_iter;
      			++surfel_result_iter; 
      			++neighbour_result_iter; 
      			++surfel_frame_iter; 
      			++neighbour_frame_iter;
    		}
  		}
  		eligible_surfel_frames.resize(surfel_result_iter - eligible_surfel_frames.begin());
  		eligible_neighbour_frames.resize(neighbour_result_iter - eligible_neighbour_frames.begin());
	}
}

/**
 * Compute a new tangent for the given surfel S
 * 
 * for each neighbouring surfel N of S
 *   find any frame f in which both N and S are visible
 *   obtain MfS the transformation matrix from frame f for S
 *   obtain dirNS from MfS * dirN
 *   perform 4RoSy smoothing operation on dirS and dirNS 
 * end
 */
Eigen::Vector3f
compute_new_tangent_for_surfel(	const std::vector<Surfel>& surfels,
    							size_t surfel_idx)
{
    using namespace Eigen;
    using namespace std;

    // Get vector of eligible normal/tangent pairs
    vector<Vector3f> normals;
    vector<Vector3f> tangents;
    get_eligible_normals_and_tangents(surfels, surfel_idx, normals, tangents);

    // Merge all neighbours; implicitly using optiminsing tier tangents
    Vector3f new_tangent = surfels.at(surfel_idx).tangent;

    float weight = 0;
    for( size_t idx = 0; idx < normals.size(); ++idx) {
        float edge_weight = 1.0f;
        new_tangent = average_rosy_vectors(new_tangent, Vector3f{0.0f, 1.0, 0.0f}, weight,
                                           tangents.at(idx), normals.at(idx),edge_weight);
        weight += edge_weight;
    }
    return new_tangent;
}





/**
 * Perform a single step of optimisation.
 */
bool
optimise_do_one_step(std::vector<Surfel>& surfels) {
    using namespace std;
    using namespace Eigen;

    // If not optimising, perform setup
    if (!g_is_optimising) {
        optimise_begin(surfels); //setup_optimisation();
    }

    // Select random surfel 
    size_t surfel_idx = random_index(surfels.size());

    // Update this one
    compute_new_tangent_for_surfel(surfels, surfel_idx);

    // Check for done-ness
	float new_error = total_error();
    bool is_converged = check_convergence(new_error);
    g_optimisation_error = new_error;
    if( is_converged ) {
        optimise_end(); // stop_optimising
    }
    return is_converged;
}

/**
 * Perform orientation field optimisation.
 * Continuously step until done.
 */
void
optimise(SurfelGraphPtr graph, std::vector<Surfel>& surfels) {
	bool done = false;
	while( !done ) {
        done = optimise_do_one_step(surfels);
    }
}
