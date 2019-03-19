#include "smooth.h"
#include <Eigen/Core>
#include <random>
#include <math.h>

static bool g_is_optimising = false;
static float g_optimisation_error;

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
