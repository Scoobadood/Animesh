#include "smooth.h"
#include <Eigen/Core>
#include <random>
#include <math.h>
#include <RoSy/RoSy.h>
#include <sys/stat.h>

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
   **             Utilities                                                      **
   **                                                                            **
   ********************************************************************************
*/
bool compareFrameDataByFrame(const FrameData &fd1, const FrameData &fd2) {
    return fd1.frame_idx < fd2.frame_idx;
}

/**
 * Sort all framedata for each surfel in ascending order of frame id.
 * We do this once to facilitate finding common frames.
 */
void 
sort_frame_data(std::vector<Surfel>& surfels) {
	for( auto surfel : surfels ) {
		sort(surfel.frame_data.begin(), surfel.frame_data.end(), compareFrameDataByFrame);
	}
}

/**
 * Check for presence of a file to see if we should stop
 */
bool 
check_stop_flag() {
  struct stat buffer;   
  return (stat ("halt", &buffer) == 0); 
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
	sort_frame_data(surfels);
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
	static int iterations = 0;
	if( iterations % 100 == 0 ) {
		std::cout << "check_convergence() " << iterations <<" iterations" << std::endl;
	}
	return (++iterations == 5000);
}

/**
 * On entry surfel and neighbour frames are sorted in ascending order of frame_id
 * common_frames is allocated. The algorithm will clear and resize it.
 * on exit, common_frames contains pairs of <surfel,neghbour> frames with matching IDs
 */
void 
find_common_frames(	const std::vector<FrameData>& surfel_frames, 
					const std::vector<FrameData>& neighbour_frames, 
					std::vector<std::pair<FrameData, FrameData>>& common_frames)
{
	using namespace std;

	unsigned long max_common_values = min(surfel_frames.size(), neighbour_frames.size());
	common_frames.clear();
	common_frames.resize(max_common_values);

	// Find the intersection of these sets
	auto surfel_frame_iter = surfel_frames.begin();
	auto neighbour_frame_iter = neighbour_frames.begin();
	auto common_iter = common_frames.begin();

	while (surfel_frame_iter != surfel_frames.end() && neighbour_frame_iter != neighbour_frames.end()) {
		if (surfel_frame_iter->frame_idx < neighbour_frame_iter->frame_idx) {
			++surfel_frame_iter;
		} else if (neighbour_frame_iter->frame_idx < surfel_frame_iter->frame_idx) {
			++neighbour_frame_iter;
		} else {
  			*common_iter = make_pair(*surfel_frame_iter, *neighbour_frame_iter);
  			++common_iter; 
  			++surfel_frame_iter; 
  			++neighbour_frame_iter;
		}
	}
  	common_frames.resize(common_iter - common_frames.begin());
}

/**
 * Populate tangents and normals with all eligible tangents, normals from surfel's neighbours
 * tan/norm is eligible iff the neighbour and surfel share a common frame
 * tan/norm are converted to the orignating surfel's frame of reference.
 */
void
get_eligible_normals_and_tangents(	const std::vector<Surfel>& surfels, 
									std::size_t surfel_idx, 
									const std::vector<std::vector<PointWithNormal>>& point_normals,
									std::vector<Eigen::Vector3f>& eligible_normals,
									std::vector<Eigen::Vector3f>& eligible_tangents) {
	using namespace std;
	using namespace Eigen;

	const vector<FrameData>& surfel_frames = surfels.at(surfel_idx).frame_data;

	// For each neighbour
	for ( size_t neighbour_idx : surfels.at(surfel_idx).neighbouring_surfels) {
		const vector<FrameData>& neighbour_frames = surfels.at(neighbour_idx).frame_data;
		vector<pair<FrameData, FrameData>> common_frames;
		find_common_frames(surfel_frames, neighbour_frames, common_frames);

		// For each common frame, get normal and tangent in surfel space
		for( auto frame_pair : common_frames) {
			const Matrix3f surfel_to_frame = frame_pair.first.transform;
			const Matrix3f frame_to_surfel = surfel_to_frame.transpose();
			const Matrix3f neighbour_to_frame = frame_pair.second.transform;
			Vector3f neighbour_normal_in_frame = point_normals.at(frame_pair.second.frame_idx).at(frame_pair.second.point_idx).normal;

			// Push the neighbour normal and tangent into the right frame
			// Transform the frame tangent back to the surfel space using inv. surfel matrix
			// So we need:
			//    transform from free space to frame space for tangent (stored) (we already have normal in frame space)
			//	  transform from frame space to free space using surfel data. (inv of stored)
			const Matrix3f neighbour_to_surfel = frame_to_surfel * neighbour_to_frame;
			Vector3f neighbour_tan_in_surfel_space = neighbour_to_surfel * surfels.at(neighbour_idx).tangent;
			Vector3f neighbour_norm_in_surfel_space = frame_to_surfel * neighbour_normal_in_frame;
			eligible_normals.push_back(neighbour_norm_in_surfel_space);
			eligible_tangents.push_back(neighbour_tan_in_surfel_space);
		}
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
    							size_t surfel_idx,
    							const std::vector<std::vector<PointWithNormal>>& point_normals)
{
    using namespace Eigen;
    using namespace std;

    // Get vector of eligible normal/tangent pairs
    vector<Vector3f> normals;
    vector<Vector3f> tangents;
    get_eligible_normals_and_tangents(surfels, surfel_idx, point_normals, normals, tangents);

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
optimise_do_one_step(std::vector<Surfel>& surfels,
					 const std::vector<std::vector<PointWithNormal>>& point_normals) 
{
    using namespace std;
    using namespace Eigen;

    // If not optimising, perform setup
    if (!g_is_optimising) {
        optimise_begin(surfels); //setup_optimisation();
    }

	auto start = chrono::high_resolution_clock::now();

    // Select random surfel 
    size_t surfel_idx = random_index(surfels.size());

    // Update this one
    compute_new_tangent_for_surfel(surfels, surfel_idx, point_normals);

	auto end = chrono::high_resolution_clock::now();
	chrono::milliseconds dtn = chrono::duration_cast<chrono::milliseconds> (end - start);
	cout << "optimise_do_one_step ran in : " << dtn.count() << endl;

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
optimise(std::vector<Surfel>& surfels,
		 const std::vector<std::vector<PointWithNormal>>& point_normals)
{
	bool done = false;
	while( !done ) {
        done = optimise_do_one_step(surfels, point_normals);
        if( !done) {
        	done = check_stop_flag();
        	if(done) {
        		std::cout << "Halted" << std::endl;
        	}
        }
    }
}
