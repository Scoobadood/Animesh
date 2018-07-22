#pragma once

#include <Eigen/Core>
#include <pcl/point_types.h>

namespace animesh {

/**
 * FieldElement stores the location, normal and tangent for each
 * element of the directional field
 */
class FieldElement {
private:
	Eigen::Vector3f		m_location;
	Eigen::Vector3f		m_normal;
	Eigen::Vector3f		m_tangent;

public:

	/**
	 * Construct a FieldElement given a pcl::PointNormal
	 * @param pointNormal The pcl::PointNormal
	 * @return Ptr to a FieldElement
	 */
	static FieldElement * from_point( const pcl::PointNormal& point );

	/**
	 * Construct a FieldElement with a given location and normal. This will generate a random tangent which 
	 * is of unit length and perpendicular to the normal.
	 * @param location The 3D location of the element in space.
	 * @param normal A unit vector in the dircetion of the normal to the point
	 */
	FieldElement( const Eigen::Vector3f& location,  const Eigen::Vector3f& normal );

	/**
	 * Construct a FieldElement with a given location, normal and tangent. 
	 * @param location The 3D location of the element in space.
	 * @param normal A unit vector in the dircetion of the normal to the point
	 * @param tangent A unit vector perpendicular to the normal
	 */
	FieldElement( const Eigen::Vector3f& location,  const Eigen::Vector3f& normal, const Eigen::Vector3f& tangent );

	/**
 	 * Useful method for merging FieldElements
	 */
	static FieldElement * mergeFieldElements ( const FieldElement * const fe1, const FieldElement * const fe2 );

	/**
 	 * Propagate field element changes down graph hierarch
	 */
	static FieldElement * propagateFieldElements ( const FieldElement * const parent, const FieldElement * const child );

	/* Getters */
	inline Eigen::Vector3f	location( ) const { return m_location;}
	inline Eigen::Vector3f	normal( ) const { return m_normal;}
	inline Eigen::Vector3f	tangent( ) const { return m_tangent;}

	/** Setters */
	void set_tangent( const Eigen::Vector3f& tangent );
};
/**
 * Write a FieldElement to a stream
 */
std::ostream& operator<<( std::ostream& os, const FieldElement& fe); 

}