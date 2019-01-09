#pragma once

#include <Eigen/Core>
#include <map>

namespace animesh {

class PointNormal {
public:
	PointNormal( Eigen::Vector3f point, Eigen::Vector3f normal);

	using Ptr=std::shared_ptr<PointNormal>;

	/**
	 * @return The point.
	 */
	inline const Eigen::Vector3f& point() { return m_point; }

	/** 
	 * @return The normal.
	 */
	inline const Eigen::Vector3f& normal() { return m_normal; }

private:
	Eigen::Vector3f 	m_point;
	Eigen::Vector3f 	m_normal;
};

}

/**
 * Compute the angle between two vectors
 */
float angle_between_vectors( Eigen::Vector3f v1, Eigen::Vector3f v2 );

/**
 * Given an arbitrary vector v, project it into the plane whose normal is given as n
 * also unitize it.
 * @param v The vector
 * @param n The normal
 * @return a unit vector in the tangent plane
 */
Eigen::Vector3f reproject_to_tangent_space( const Eigen::Vector3f& v, const Eigen::Vector3f& n);

/**
 * Construct the skew symmetric matrix correesponding to 
 * the vector (v1,v2,v3)
 */
Eigen::Matrix3f skew_symmetrix_matrix_for( const Eigen::Vector3f& v );

/**
 * Compute the vector v resulting from the rotation of vector o around normal n
 * through 2*pi*k/4
 *
 * @param o The vector to be rotated
 * @param n The normal around which to rotate o
 * @param k The integral multiplier rotation (0-3)
 * @return The rotated vector 
 */
 Eigen::Vector3f vector_by_rotating_around_n( const Eigen::Vector3f & o, const Eigen::Vector3f & n, int k);

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
 									const std::vector<Eigen::Vector3f>& neighbours2);

 /**
 * Compute the matrix which rotates an arbitrary 3D vector onto another.
 * @param v1 The first vector
 * @param v2 The second vector
 * @return The 3x3 rotation matrix
 */
Eigen::Matrix3f vector_to_vector_rotation( const Eigen::Vector3f& v1, const Eigen::Vector3f& v2 );

/**
 * Return a 3D vector perpendicular to the given 3D vector
 * @param v The vector
 * @retrun A vector perpendicular to it
 */
Eigen::Vector3f vector_perpendicular_to_vector( const Eigen::Vector3f& v1 );