#include <Eigen/Geometry>
#include <Geom/geom.h>
#include <vector>
#include <iostream>

const float EPSILON = 1e-4;


/**
 * Compute the angle between two vectors
 */
float angle_between_vectors( Eigen::Vector3f v1, Eigen::Vector3f v2 ) {
	using namespace Eigen;

	return std::acos(std::min(1.0f, v1.dot(v2))) * 180 / M_PI;
}


/**
 * Construct the skew symmetric matrix correesponding to
 * the vector (v1,v2,v3)
 */
Eigen::Matrix3f skew_symmetrix_matrix_for( const Eigen::Vector3f& v ) {
	Eigen::Matrix3f m;
	m << 0, -v(2) , v(1), v(2), 0, -v(0), -v(1), v(0), 0;

	return m;
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

	if ( fabs( o.norm() ) < EPSILON )
		throw std::invalid_argument( "Vector may not be zero length" );

	if ( fabs( n.norm() - 1.0f ) > EPSILON )
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
	Vector3f reprojected = v - error;
	reprojected.normalize();
	return reprojected;
}


/**
 * Return a 3D vector perpendicular to the given 3D vector
 * @param v The vector
 * @retrun A vector perpendicular to it
 */
Eigen::Vector3f vector_perpendicular_to_vector( const Eigen::Vector3f& v ) {
	using namespace Eigen;

	// Assert that is got a length
	if ( v.norm() < EPSILON )
		throw std::invalid_argument( "Vector may not be zero length" );

	if ( (v[0] < v[1]) && (v[0] < v[2]) )
		return v.cross( Vector3f(1, 0, 0) );

	if ( (v[1] <= v[0]) && (v[1] < v[2]) )
		return v.cross( Vector3f(0, 1, 0) );

	return v.cross( Vector3f(0, 0, 1) );
}

/**
 * Compute the matrix which rotates an arbitrary 3D vector onto another.
 * @param v1 The first vector
 * @param v2 The second vector
 * @return The 3x3 rotation matrix
 */
Eigen::Matrix3f vector_to_vector_rotation( const Eigen::Vector3f& v1, const Eigen::Vector3f& v2 ) {
	using namespace Eigen;

	Matrix3f ret;

	// Sanity check
	assert(v1.norm( ) >= EPSILON && v2.norm( ) >= EPSILON );

	// First compute the axis of rotation by finding the cross product
	Vector3f vu1 = v1.normalized( );
	Vector3f vu2 = v2.normalized( );

	// Handle the case where the vectors are the same direction - return identity
	float cos_alpha = vu1.dot( vu2 );
	if ( abs(cos_alpha - 1.0f) < EPSILON)
		return Matrix3f::Identity( );

	// If they're opposite directions then pick an arbitrary axis perp to both and rotate by pi
	if ( abs(cos_alpha + 1.0f) < EPSILON ) {
		Vector3f axis = vector_perpendicular_to_vector( vu1 ).normalized();
		// Compute rotation through pi about this axis
		// From http://scipp.ucsc.edu/~haber/ph216/rotation_12.pdf (p5)
		float ax = axis.x(), ay = axis.y(), az = axis.z();
		ret <<  2 * ax*ax - 1, 2 * ax*ay,	  2 * ax*az,
		      2 * ay*ax,	 2 * ay*ay - 1, 2 * ay*az,
		      2 * az*ax,	 2 * az*ay,	  2 * az*az - 1;
		return ret;
	}

	// In the general case, compute the axis (perp to both vectors)
	Vector3f axis = vu1.cross( vu2 );

	// Now construct R as I + [v]_x + [v]_x^2 /(1+c)
	Matrix3f v_x = skew_symmetrix_matrix_for( axis );
	Matrix3f v_x2 = v_x * v_x;
	v_x2 = v_x2 / (1 + cos_alpha );

	ret = Matrix3f::Identity() + v_x + v_x2;
	return ret;
}

/**
 * Compute the best rotation to map the given normal N1 with neighbouring points P to N2 with neighbouring points P2
 * Such that the normals are coincident and the position of the neighbours are as close as possible to their
 * actual position.
 */
Eigen::Matrix3f rotation_between(	const Eigen::Vector3f& point1,
                                    const Eigen::Vector3f& normal1,
                                    const std::vector<Eigen::Vector3f>& neighbours1,
                                    const Eigen::Vector3f& point2,
                                    const Eigen::Vector3f& normal2,
                                    const std::vector<Eigen::Vector3f>& neighbours2) {
	using namespace Eigen;
	assert( neighbours2.size() == neighbours1.size() );

	// First compute the rotations which bring the two normal vectors into alignment with z-axis
	Vector3f z_axis{ 0, 0, 1};
	Matrix3f vvrN1 = vector_to_vector_rotation( normal1, z_axis );
	Matrix3f vvrN2 = vector_to_vector_rotation( normal2, z_axis );


	float theta = 0.0f;
	if ( neighbours1.size() > 0 ) {
		float pxqx = 0.0f;
		float pyqy = 0.0f;
		float pxqy = 0.0f;
		float pyqx = 0.0f;

		// For each point in neighbours1 and corresponding point in neighbours2
		size_t num_neighbours = neighbours1.size();
		for ( size_t i = 0; i < num_neighbours; ++i ) {
			Vector3f p = neighbours1[i];
			Vector3f q = neighbours2[i];

			// Translate both such that rotation is around origin
			p = p - point1;
			q = q - point2;

			// Put in common space
			p = vvrN1 * p;
			q = vvrN2 * q;

			// Update  products of X and Y coords
			pxqx += (p[0] * q[0]);
			pxqy += (p[0] * q[1]);
			pyqx += (p[1] * q[0]);
			pyqy += (p[1] * q[1]);
		}

		// tan(theta) = -(qxpy - qypx) / (pxqx+pyqy)
		float A = pxqy - pyqx;
		float B = pxqx + pyqy;
		theta = atan2( A, B );
	}

	Matrix3f in_plane_rot;
	in_plane_rot << std::cos( theta ), -std::sin( theta ), 0, std::sin( theta) , std::cos(theta), 0, 0, 0, 1;

	// Total transformation is vvrN2' * in_plane_rot * vvrN1
	Matrix3f result = vvrN2.inverse() * in_plane_rot * vvrN1;
	for ( size_t i = 0; i < 9; ++i ) {
		assert( !isnan(result(i)));
		assert( !isinf(result(i)));
	}
	return result;
}