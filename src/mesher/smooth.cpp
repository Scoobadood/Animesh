#include "smooth.h"
#include <Eigen/Core>
#include <random>
#include <math.h>

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
smooth(std::vector<Surfel>& surfels) {

}