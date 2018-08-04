#include <Field/CorrespondenceMapping.h>
#include <vector>
#include <Geom/geom.h>


using animesh::CorrespondenceMapping;
using animesh::FieldGraph;
using animesh::FieldElement;

/**
 * Construct the correspondence mapping between these two sets of FEs.
 * Assumptions are that the vectors are the same size and the elements in them
 * are ordered s.t. corresponding elements are at the same position in the vector.
 * The grap describes the connectivity between the elements and is used to determine
 * transformations.
 */
CorrespondenceMapping::CorrespondenceMapping(std::vector<FieldElement*> first,
        std::vector<FieldElement*> second,
        FieldGraph * graph) {

	using namespace std;
	using namespace Eigen;

	assert(first.size() > 0 );
	assert(second.size() == first.size() );
	assert(graph->num_nodes() == first.size());

	for (size_t idx = 0; idx < first.size(); ++idx) {
		m_mapping.insert(make_pair(first[idx], make_pair(second[idx], Matrix3f::Identity())));
	}

	for (auto gn : graph->nodes()) {
		// 1. Get the neighbours of this FE according to graph
		FieldElement *fe = gn->data();
		vector<FieldElement *> neighbour_fes = graph->neighbours_data(gn);

		// First point and normal
		Vector3f point1 = fe->location();
		Vector3f normal1 = fe->normal();

		// Second point and normal
		const FieldElement * other_fe = get_corresponding_fe( fe );
		Vector3f point2 = other_fe->location();
		Vector3f normal2 = other_fe->normal();

		// Find points for neighbours at both time intervals
		vector<Vector3f> neighbour_points_1;
		vector<Vector3f> neighbour_points_2;
		for (auto fen : neighbour_fes) {
			neighbour_points_1.push_back(fe->location());
			neighbour_points_2.push_back(m_mapping.at(fen).first->location());
		}

		Matrix3f m = rotation_between(point1, normal1, neighbour_points_1, point2, normal2, neighbour_points_2);
		m_mapping[fe].second = m;
	}
}

/**
 * @return the FE corresponding to the input FE
 */
const FieldElement *
CorrespondenceMapping::get_corresponding_fe( const FieldElement * src_fe ) const {
	assert( src_fe != nullptr );
	auto it = m_mapping.find(const_cast<FieldElement*>(src_fe));
	assert( it != m_mapping.end());
	return it->second.first;
}

/**
 * @return the Matrix transformation from the input FE to it's corresponding FE
 */
const Eigen::Matrix3f& 
CorrespondenceMapping::get_transformation_for( const FieldElement * src_fe ) const {
	assert( src_fe != nullptr );
	auto it = m_mapping.find(const_cast<FieldElement*>(src_fe));
	assert( it != m_mapping.end());
	return it->second.second;
}
