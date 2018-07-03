#include <Args/Args.h>
#include <Field/Field.h>
#include <Graph/GraphBuilder.h>
#include <fstream>
#include <sstream>
#include <vector>

const std::string OUTPUT_DIRECTORY = "/Users/dave/Desktop/animesh_output";

/**
 * Write to obj file
 */
void write_field_to_obj_file( std::string file_name, Field * field ) {
	// Open file
	std::ostringstream oss;
	oss << OUTPUT_DIRECTORY << "/" << file_name;

	std::ofstream file{ oss.str() };

	std::vector<const FieldElement *> elements = field->elements();
	for( auto iter = elements.begin(); iter != elements.end(); ++iter) {
		const FieldElement * fe = *iter;
		// Write vertices
		file << "v " << fe->m_location[0] << " " << fe->m_location[1] << " " << fe->m_location[2] << std::endl;
	}
	// Write Normals
	for( auto iter = elements.begin(); iter != elements.end(); ++iter) {
		const FieldElement * fe = *iter;
		// Write vertices
		file << "vn " << fe->m_normal[0] << " " << fe->m_normal[1] << " " << fe->m_normal[2] << std::endl;
	}
	// Write face data which links vertices and norms
	file << "# These are _not_ faces, just a vehicle for associating normals with vertices" << std::endl;
	for( std::size_t i=0; i < elements.size(); ++i ) {
		// Write pseudo faces
		if( i % 3 == 0 ) 
			file << "f " << i << "//" << i;
		else
			file << i << "//" << i;

		if( i % 3 == 2) 
			file << std::endl;
		else 
			file << " ";
	}
}

/**
 * Main entry point
 */
int main( int argc, char * argv[] ) {
	Args args{ argc, argv};

	// Make sphere
	float radius = 20.0f;
	std::size_t theta_steps = 20;
	std::size_t phi_steps = 20;
	int k = 4;
	bool make_fixed = false;
	Field * field = Field::spherical_field( radius, theta_steps, phi_steps, k);
	write_field_to_obj_file( "sphere.obj", field );

	// Make plane

	// Make polynomial

	// Make cube

    return EXIT_SUCCESS;
}