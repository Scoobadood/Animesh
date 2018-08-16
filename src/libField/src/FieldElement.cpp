#include <Field/FieldElement.h>
#include <Geom/geom.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <pcl/point_types.h>

#define throw_invalid_argument(msg) \
    throw std::invalid_argument(msg " at " __FILE__ ":" + std::to_string(__LINE__))

static const float EPSILON = 1e-4;

using animesh::FieldElement;

/**
 * Check that a vector is of unit length or throw
 */
void checkUnitLength( std::string vector_name, Eigen::Vector3f vector ) {
	using namespace std;

	assert ( abs(vector.norm() - 1.0) <= EPSILON );
	if ( abs(vector.norm() - 1.0) > EPSILON ) {
		stringstream buffer;
		buffer << vector_name << " [" << vector[0] << ", " << vector[1] << ", " << vector[2] << "] is not unit length";
		throw invalid_argument(buffer.str() + " at " __FILE__ ":" + to_string(__LINE__));
	}
}

/**
 * Check that two vectors are perpendicular or throw
 */
void checkPerpendicular( std::string vec1_name, Eigen::Vector3f vec1, std::string vec2_name, Eigen::Vector3f vec2 ) {
	using namespace std;
	
	assert ( abs(vec1.dot( vec2) ) <= EPSILON );
	if ( abs(vec1.dot( vec2) ) > EPSILON ) {
		stringstream buffer;
		buffer << vec1_name << " [" << vec1[0] << ", " << vec1[1] << ", " << vec1[2] << "] and " << vec2_name << " [" << vec2[0] << ", " << vec2[1] << ", " << vec2[2] << "] are not perpendicular";

		throw invalid_argument(buffer.str() + " at " __FILE__ ":" + to_string(__LINE__));
	}
}

/**
 * Construct a FieldElement given a pcl::PointNormal
 * @param pointNormal The pcl::PointNormal
 * @return Ptr to a FieldElement
 */
FieldElement * FieldElement::from_point( const pcl::PointNormal& point ) {
	Eigen::Vector3f location{ point.x, point.y, point.z};
	Eigen::Vector3f normal { point.normal_x, point.normal_y, point.normal_z};

	checkUnitLength("Normal", normal);

	FieldElement * fe = new FieldElement( location, normal);
	return fe;
}

/**
 * Construct a FieldElement with a given location and normal. This will generate a random tangent which
 * is of unit length and perpendicular to the normal.
 * @param location The 3D location of the element in space.
 * @param normal A unit vector in the dircetion of the normal to the point
 */
FieldElement::FieldElement( const Eigen::Vector3f& location,  const Eigen::Vector3f& normal ) {
	using namespace std;

	// Preconditions : Normal is unit length
	checkUnitLength("Normal", normal);

	m_location = location;
	m_normal = normal;

	Eigen::Vector3f random = Eigen::Vector3f::Random();
	m_tangent = (random.cross( normal )).normalized();
}

/**
 * Construct a FieldElement with a given location, normal and tangent.
 * @param location The 3D location of the element in space.
 * @param normal A unit vector in the dircetion of the normal to the point
 * @param tangent A unit vector perpendicular to the normal
 */
FieldElement::FieldElement( const Eigen::Vector3f& location,  const Eigen::Vector3f& normal, const Eigen::Vector3f& tangent ) {
	using namespace std;

	// Preconditions : Normal and tangent are perpendicular and neither is 0
	checkUnitLength("Normal", normal);
	checkUnitLength("Tangent", tangent);
	checkPerpendicular("Tangent", tangent, "Normal", normal);

	m_location = location;
	m_normal = normal;
	m_tangent = tangent;
}

/**
 * Merge FieldElements when simplifying a graph. Each FieldElement has a normal, tangent and location.
 * When merging we do the following:
 * Locations are averaged
 * Normals are averaged
 * Tangents are randomised, made perpendicular to the normal and unitised
 */
FieldElement * FieldElement::mergeFieldElements ( const FieldElement * const fe1, const FieldElement * const fe2 ) {
	using namespace Eigen;

	Vector3f new_location = (fe1->m_location + fe2->m_location) / 2.0;
	Vector3f new_normal   = (fe1->m_normal + fe2->m_normal).normalized();
	Vector3f new_tangent  = Eigen::Vector3f::Random().cross( new_normal ).normalized();

	FieldElement *fe = new FieldElement( new_location, new_normal, new_tangent );
	return fe;
}

/**
 * When simplifying the graph, propagate the changes from parent to child.
 * by just copying it.
 */
FieldElement * FieldElement::propagateFieldElements ( const FieldElement * const parent, const FieldElement * const child ) {
	using namespace Eigen;

	// Take the tangent from the parent and reproject into the child's tangent space and normalise
	Vector3f new_tangent = reproject_to_tangent_space( parent->m_tangent, child->m_normal);
	FieldElement * new_element = const_cast<FieldElement*>(child);
	new_element->set_tangent(new_tangent);
	return new_element;
}


/**
 * Set the tangent.
 * @parameter A tangent which must be unit length and perpendicular to the normal.
 */
void FieldElement::set_tangent( const Eigen::Vector3f& tangent ) {
	using namespace std;

	checkUnitLength( "Tangent", tangent );
	checkPerpendicular("Tangent", tangent, "Normal", m_normal);
	m_tangent = tangent;
}

/**
 * Write a FieldElement to a stream
 */
/**
 * Write a FieldElement to output stream;
 */
std::ostream& 
animesh::operator<<( std::ostream& os, const FieldElement& fe) {
	os	<< "l=( "
	    << fe.location()[0] << ", "
	    << fe.location()[1] << ", "
	    << fe.location()[2] << "), n=("
	    << fe.normal()[0] << ", "
	    << fe.normal()[1] << ", "
	    << fe.normal()[2] << "), t=("
	    << fe.tangent()[0] << ", "
	    << fe.tangent()[1] << ", "
	    << fe.tangent()[2] << ")";
	return os;
}