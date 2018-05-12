
#include "load_and_save.h"
#include "visualisation.h"

/**
 * Main entry point
 */
int main( int argc, char * argv[] ) {
	Args args{ argc, argv};

	Field * field = load_field( args );

	start_renderer( field );

	delete field;
    return EXIT_SUCCESS;
}
