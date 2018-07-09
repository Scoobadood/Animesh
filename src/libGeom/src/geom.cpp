#include <Eigen/Geometry>
#include <Geom/geom.h>

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
 * COmpute the best rotation to map the given normal N1 with neighbouring points P to N2 with neighbouring points P2
 * Such that the normals are coincident and the position of the neghbours are as close as possible to their
 * actual position.
 */
 Eigen::Matrix3f rotation_between(	const Eigen::Vector3f& point1, 
 									const Eigen::Vector3f& normal1, 
 									const std::vector<Eigen::Vector3f>& neighbours1,
 									const Eigen::Vector3f& point2, 
 									const Eigen::Vector3f& normal3, 
 									const std::vector<Eigen::Vector3f>& neighbours2) {
 	
 }
 