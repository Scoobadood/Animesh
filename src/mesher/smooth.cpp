#include "smooth.h"
#include <Eigen/Core>
#include <random>
#include <math.h>
#include <Graph/Graph.h>
#include <Graph/GraphSimplifier.h>

static bool g_is_optimising = false;
static float g_optimisation_error;

using animesh::Graph;
using animesh::GraphSimplifier;
using SurfelGraph    = Graph<std::size_t, void *>;
using SurfelGraphNode = Graph<std::size_t, void *>::GraphNode;
using SurfelGraphPtr = std::shared_ptr<SurfelGraph>;
using SurfelGraphSimplifier = GraphSimplifier<std::size_t, void *>;
using SurfelGraphMapping = SurfelGraphSimplifier::GraphMapping;

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

    // Do the optimisation
    // Select random surfel 
    // Smooth 

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
optimise(std::vector<Surfel>& surfels) {
	bool done = false;
	while( !done ) {
        done = optimise_do_one_step(surfels);
    }
}
