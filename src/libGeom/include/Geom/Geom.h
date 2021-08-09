#pragma once

#include <Eigen/Core>
#include <map>
#include <memory>
#include <vector>

namespace animesh {

class PointNormal {
public:
	PointNormal( const Eigen::Vector3f& point, const Eigen::Vector3f& normal);

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
float degrees_angle_between_vectors(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2 );

/**
 * Given an arbitrary vector v, project it into the plane whose normal is given as n
 * also unitize it.
 * @param v The vector
 * @param n The normal
 * @return a unit vector in the tangent plane
 */
Eigen::Vector3f project_vector_to_plane(const Eigen::Vector3f& v, const Eigen::Vector3f& n, bool normalize = true);

/**
 * Rotate a point about an axis through an angle.
 * @param axis The axis.
 * @param angle The angle in radians.
 * @param p The point to rotate.
 * @return The rotated point.
 */
Eigen::Vector3f rotate_point_through_axis_angle(
        const Eigen::Vector3f& axis,
        float theta,
        const Eigen::Vector3f& p);



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
 * @return A vector perpendicular to it
 */
Eigen::Vector3f vector_perpendicular_to_vector( const Eigen::Vector3f& v1 );

/**
 * Return a 3D vector representing the centroid of the given vector of points
 * @param points A vector containing at least one point (asserted)
 * @return The centroid of those points.
 */
Eigen::Vector3f compute_centroid(const std::vector<Eigen::Vector3f>& points);

/**
 * Return a 3D vector representing the centroid of the given vector of points
 * @param xyz A vector containing X, Y and Z coordinates of a set of points. Must have length multiple of 3 and at least 3
 * Populates centroidX centroidY and centroidZ
 */
void compute_centroid(const std::vector<float>& xyz,
                      float& centroidX,
                      float& centroidY,
                      float& centroidZ);

/**
 * Compute the perpendicular distance between a point and a line given.
 * @param point The point in 3D space.
 * @param anchor A point on the line.
 * @param direction A vector indicating the direction of the line.
 * @return The perpendicular distance between the point and the line.
 */
float distance_from_point_to_line(const Eigen::Vector3f &point, const Eigen::Vector3f &anchor,
                                  const Eigen::Vector3f &direction);

/**
 * Return the min and max X,Y and Z for the given points.
 * @param xyz A vector containing X, Y and Z coordinates of a set of points. Must have length multiple of 3 and at least 3
 * Populates minX, maxX etc.
 */
void compute_bounds( const std::vector<float>& xyz,
                     float& minX, float& maxX,
                     float& minY, float& maxY,
                     float& minZ, float& maxZ);

/**
 * Find the closest pair of points in a given cloud.
 * Return the indices of the points in the input vector.
 * Based on https://www.researchgate.net/publication/300206787_Time-Optimal_Heuristic_Algorithms_for_Finding_Closest-Pair_of_Points_in_2D_and_3D/fulltext/571400c908aeff315ba35895/Time-Optimal-Heuristic-Algorithms-for-Finding-Closest-Pair-of-Points-in-2D-and-3D.pdf
 * Should run in O(nlogn)
 * @parap points
 * @return tuple of indices and distance between points
 */
std::tuple<unsigned int, unsigned int, float> closest_points(const std::vector<Eigen::Vector3f>& points);

/**
 * @return true if the given vector is close to zero length.
 */
bool
is_zero_vector(const Eigen::Vector3f& vector);

/**
 * @return true if the given vector is close to unit length.
 */
bool
is_unit_vector(const Eigen::Vector3f& vector);

/**
 * @return The Euclidean distance between the two points.
 */
float
distance_from_point_to_point(const Eigen::Vector3f& p1, const Eigen::Vector3f& p2 );

/**
 * @return The Euclidean distance between the two points.
 */
float
distance_from_point_to_point(
        float p1x, float p1y, float p1z,
        float p2x, float p2y, float p2z);

/**
 * Convert from polar to cartesian coordinates.
 *
 * @param azimuth Rotation in the XY plane. [0, 2 * pi)
 * @param inclination Vertical rotation. [0, pi)
 * @param radius Distance from the sphere centre.
 * @return X,Y Z coordinates.
 */
Eigen::Vector3f
spherical_to_cartesian(float radius, float azimuth, float inclination );