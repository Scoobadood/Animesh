#pragma once

#include <Field/Field.h>
#include <Graph/Graph.h>
#include <Field/FieldElement.h>
#include <Eigen/Core>
#include <map>

namespace animesh {
class CorrespondenceMapping {
public:
	/**
	 * Construct the correspondence mapping between these two sets of FEs.
	 * Assumptions are that the vectors are the same size and the elements in them
	 * are ordered s.t. corresponding elements are at the same position in the vector.
	 * The grap describes the connectivity between the elements and is used to determine
	 * transformations.
	 */
	CorrespondenceMapping(std::vector<FieldElement*> first, std::vector<FieldElement*> second, FieldGraph * graph);

	/**
	 * @return the FE corresponding to the input FE
	 */
	const FieldElement * get_corresponding_fe( const FieldElement * src_fe ) const;

	/**
	 * @return the Matrix transformation from the input FE to it's corresponding FE
	 */
	const Eigen::Matrix3f& get_transformation_for( const FieldElement * src_fe ) const;
private:
	/** Store the mapping */
	std::map<FieldElement *, std::pair<FieldElement *, Eigen::Matrix3f>> m_mapping;
};
}