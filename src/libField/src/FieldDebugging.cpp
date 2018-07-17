#include <FIeld/Field.h>
#include <vector>
#include <iostream>


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
	for( auto gn :  m_graph_hierarchy[0]->nodes()) {

		FieldElement *fe = gn->data();

		std::cout << "locn    (" << fe->m_location[0] << "," << fe->m_location[1] << "," << fe->m_location[2] << ")" << std::endl;
		std::cout << "tangent (" << fe->m_tangent[0] << "," << fe->m_tangent[1] << "," << fe->m_tangent[2] << std::endl;
		std::cout << "neighbours " << std::endl;

		std::vector<FieldGraphNode *> neighbours = m_graph_hierarchy[0]->neighbours( gn );
		for( auto neighbour_iter  = neighbours.begin();
			      neighbour_iter != neighbours.end();
			      ++neighbour_iter ) {
			FieldElement * fe_n = (*neighbour_iter)->data();
			std::cout << "      " << fe_n->m_location[0] << "," << fe_n->m_location[1] << "," << fe_n->m_location[2] << std::endl;
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
 * Write a FieldElement to output stream;
 */
std::ostream& operator<<( std::ostream& os, const FieldElement& fe) {
	os	<< "( l=( " 
		<< fe.m_location[0] << ", "   
 		<< fe.m_location[1] << ", "   
 		<< fe.m_location[2] << "), t=(" 
		<< fe.m_tangent[0] << ", "   
 		<< fe.m_tangent[1] << ", "   
 		<< fe.m_tangent[2] << ")";
 	return os;
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

