#include <iostream>
#include <Eigen/Geometry>
#include <RoSy/RoSy.h>
#include <Field/Field.h>

const float EPSILON = 1e-6;

/**
 * Compute the angle between two vectors
 */
float angle_between_vectors( Eigen::Vector3f v1, Eigen::Vector3f v2 ) {
	using namespace Eigen;

    return std::acos(std::min(1.0f, v1.dot(v2))) * 180 / M_PI;
}

/**
 * Compute the vector v resulting from the rotation of vector o around normal n
 * through 2*pi*k/4
 *
 * @param o The vector to be rotated
 * @param n The normal around which to rotate o
 * @param k The integral multiplier rotation (0-3)
 * @return The rotated vector 
 */
 Eigen::Vector3f vector_by_rotating_around_n( const Eigen::Vector3f & o, const Eigen::Vector3f & n, int k) {
 	using namespace Eigen;

 	if( fabs( o.norm() ) < EPSILON )
 		throw std::invalid_argument( "Vector may not be zero length" );
 	
 	if( fabs( n.norm() - 1.0f ) > EPSILON )
 		throw std::invalid_argument( "Normal must be unit vector" );

    return ((k & 1) ? (n.cross(o)) : o) * (k < 2 ? 1.0f : -1.0f);
 } 

/**
 * Given an arbitrary vector v, project it into the plane whose normal is given as n
 * also unitize it.
 * @param v The vector
 * @param n The normal
 * @return a unit vector in the tangent plane
 */
Eigen::Vector3f reproject_to_tangent_space( const Eigen::Vector3f& v, const Eigen::Vector3f& n) {
	using namespace Eigen;

	Vector3f error = v.dot( n ) * n;
	return (v - error);
}


/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
std::pair<Eigen::Vector3f, Eigen::Vector3f> best_rosy_vector_pair( const Eigen::Vector3f& target_vector, const Eigen::Vector3f& target_normal, 
						  	  									 const Eigen::Vector3f& source_vector, const Eigen::Vector3f& source_normal ) {
	using namespace Eigen;

	// We'll compare 0 and 90 degree rotations of each vector
	const Vector3f target_candidates[2] = { target_vector, target_normal.cross(target_vector) };
    const Vector3f source_candidates[2] = { source_vector, source_normal.cross(source_vector) };


	float best_dot_product	= -std::numeric_limits<float>::infinity();;
	int best_target_idx = 0;
	int best_source_idx = 0;

	for( int i = 0; i<2; ++i ) {
		for( int j = 0; j < 2; ++j ) {

			float dp =  std::abs( target_candidates[i].dot( source_candidates[j] ) );
			if( dp > best_dot_product ) {
				best_dot_product = dp;
				best_target_idx = i;
				best_source_idx = j;
			}
		}
	}

    const float dp = target_candidates[best_target_idx].dot(source_candidates[best_source_idx]);
    return std::make_pair(target_candidates[best_target_idx], source_candidates[best_source_idx] * std::copysign( 1.0f, dp ));
}