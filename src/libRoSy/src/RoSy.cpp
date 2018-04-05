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

	// if( (v1[0] == 0.0f && v1[1] == 0.0f && v1[2] == 0.0f ) ||
	// 	(v2[0] == 0.0f && v2[1] == 0.0f && v2[2] == 0.0f ) ) {
	// 	throw std::invalid_argument( "Vector may not be zero length" );
	// }


	// Vector3f reference{ -1.0f, -2.0f, 3.0f };
	// Vector3f      c = v1.cross(v2);
 //    float     angle = std::atan2(c.norm(), v1.dot(v2));
 //    return c.dot(reference) < 0.0f ? (2 * M_PI -angle) : angle;


    return std::acos(std::min(1.0f, v1.dot(v2))) * 180 / M_PI;


	/*
		Code below was replaced by the arccos and triple product code
		On the basis of this:
		https://www.gamedev.net/forums/topic/503639-angle-between-3d-vectors/

	// Compute the angle between the vectors using 
	// θ=2 atan2(|| ||v||u−||u||v ||, || ||v||u+||u||v ||)
	Eigen::Vector3f vu = v1.norm() * v2;
	Eigen::Vector3f uv = v2.norm() * v1;
	float theta = 2 * atan2( (vu - uv).norm(), (vu + uv).norm() );	

	return theta;
	*/
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

 	float angle = k * 0.5f * M_PI;
 	AngleAxis<float> aa{ angle , n };
 	Quaternionf q{ aa };

	return q * o;
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
 * Return vector of elements */
const std::vector<const FieldElement *> Field::elements( ) const {
	std::vector<const FieldElement *> elements;
	for( auto node_iter = m_graph->m_data_to_node_map.begin(); node_iter != m_graph->m_data_to_node_map.end(); ++node_iter ) {
		const FieldElement * fe = (FieldElement *) (*node_iter).first;

		elements.push_back( fe );
	}
	return elements;
}

/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
Eigen::Vector3f best_dp_rosy_vector_for( const Eigen::Vector3f& targetVector, 
									  const Eigen::Vector3f& targetNormal, 
									  int targetK, 
									  const Eigen::Vector3f& sourceVector, 
									  const Eigen::Vector3f& sourceNormal ) {
	using namespace Eigen;

	Vector3f effectiveTarget = vector_by_rotating_around_n( targetVector, targetNormal, targetK );
	Vector3f best{ sourceVector };
	float bestDotProduct = effectiveTarget.dot( sourceVector );

	for( int k=1; k<4; ++k ) {
		Vector3f testVector = vector_by_rotating_around_n( sourceVector, sourceNormal, k );

		float dp = effectiveTarget.dot( testVector );
		if( dp > bestDotProduct) {
			bestDotProduct = dp;
			best = testVector;
		}
	}
	return best;
}

/**
 * @param targetVector The vector we're trying to match
 * @param normal The normal about which to rotate the sourceVector
 * @param sourceVector the vector to be matched
 * @return the best fitting vector (i.e. best multiple of PI/2 + angle)
 * 
 */
Eigen::Vector3f best_rosy_vector_for( const Eigen::Vector3f& target_vector, 
									  const Eigen::Vector3f& target_normal, 
									  const Eigen::Vector3f& source_vector, 
									  const Eigen::Vector3f& source_normal,
									  int& k_ij) {
	using namespace Eigen;

	Vector3f best_vector = source_vector;
	float best_theta = angle_between_vectors( target_vector, source_vector );
	int best_k = 0;

	for( int k=1; k<4; ++k ) {
		Vector3f test_vector = vector_by_rotating_around_n( source_vector, source_normal, k );
		float theta = angle_between_vectors( target_vector, test_vector );

		if( theta < best_theta ) {
			best_theta = theta;
			best_vector = test_vector;
			best_k = k;
		}
	}
	k_ij = best_k;
	return best_vector;
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