#include <Field/Field.h>
#include <vector>
#include <iostream>

namespace animesh {

/* ******************************************************************************************
 * **
 * ** Debugging and Printing  
 * **
 * ******************************************************************************************/

/**
 * Dump the field to stdout. For each point in the field dump the neighbour locations
 * and the field tangent
 */
void Field::dump(  ) const {
	for( auto gn :  m_graph->nodes()) {

		FieldElement *fe = gn->data();

		std::cout << fe << std::endl;
		std::cout << "neighbours " << std::endl;

		std::vector<FieldGraphNode *> neighbours = m_graph->neighbours( gn );
		for( auto neighbour : neighbours ) {
			std::cout << "      " << neighbour->data()->location() << std::endl;
		}
	}
}

void Field::trace_node( const std::string& prefix, const FieldElement * this_fe ) const {
	std::cout << prefix << this_fe << std::endl;
}

void Field::trace_vector( const std::string& prefix, const Eigen::Vector3f& vector ) const {
	std::cout << prefix << vector << std::endl;
}

/**
 * Write a Vector3f to output stream;
 */
std::ostream& operator<<( std::ostream& os, const Eigen::Vector3f& vector) {
	os  << vector[0] << ", "   
	 	<< vector[1] << ", "   
	 	<< vector[2] << ")";
 	return os;
}

}