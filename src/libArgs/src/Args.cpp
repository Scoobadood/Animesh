#include <Args/Args.h>
#include <tclap/CmdLine.h>

const int DEFAULT_SMOOTH_ITERATIONS = 50;
const int DIM_X = 20;
const int DIM_Y = 20;
const float GRID_SPACING = 1.0f;
const float RADIUS = 10.0f;
const int SPHERE_THETA_STEPS = 30;
const int SPHERE_PHI_STEPS = 15;
const int CUBE_SIZE = 3;

Args::Args( int &argc, char **argv) {
	using namespace TCLAP;

	try {
		// Define the command line object, and insert a message
		// that describes the program. The "Command description message" 
		// is printed last in the help text. The second argument is the 
		// delimiter (usually space) and the last one is the version number. 
		// The CmdLine object parses the argv array based on the Arg objects
		// that it contains. 
		CmdLine cmd("Command description message", ' ', "0.9");

		// Define a value argument and add it to the command line.
		// A value arg defines a flag and a type of value that it expects,
		// such as "-n Bishop".
		ValueArg<int> iterations("i","iter","Number of smoothing iterations to run",false, DEFAULT_SMOOTH_ITERATIONS, "int", cmd);


		std::vector<std::string> allowed_shapes;
		allowed_shapes.push_back("plane");
		allowed_shapes.push_back("cube");
		allowed_shapes.push_back("sphere");
		ValuesConstraint<std::string> allowed_constraint( allowed_shapes );
		ValueArg<std::string> shape("s","shape","Field shape to use. plane, cube or sphere",false, "plane", &allowed_constraint, cmd);

		SwitchArg make_fixed("f","fix","Make initial field tangents fixed", cmd, false);
		SwitchArg dump_field("d","dump","Dump the field in it's initial state", cmd, false);
		SwitchArg tracing_enabled("v","verbose","Output lots of diagnostics", cmd, false);

		// Plane
		ValueArg<float> grid_spacing("g","grid","Spacing of grid. Only valid with plane",false, GRID_SPACING, "float", cmd);
		ValueArg<int> plane_x("x","plane_x","X dimension of plane",false, DIM_X, "int", cmd);
		ValueArg<int> plane_y("y","plane_y","Y dimension of plane",false, DIM_Y, "int", cmd);

		// Sphere
		ValueArg<float> radius("r","radius","Radius of sphere",false, RADIUS, "float", cmd);
		ValueArg<int> theta("t","theta_steps","Number of steps around sphere",false, SPHERE_THETA_STEPS, "float", cmd);
		ValueArg<int> phi("p","phi_steps","Number of steps vertically on sphere",false, SPHERE_PHI_STEPS, "float", cmd);

		// Cube
		ValueArg<int> cube_size("c","cube_size","Dimensions of cube",false, CUBE_SIZE, "int", cmd);

		// Parse the argv array.
		cmd.parse( argc, argv );

		m_num_smoothing_iterations = iterations.getValue();
		m_should_fix_tangents      = make_fixed.getValue();
		m_should_dump_field        = dump_field.getValue();
		m_tracing_enabled          = tracing_enabled.getValue();

		if( shape.getValue() == "plane") {
			m_default_shape = PLANE;
			m_plane_x = plane_x.getValue();
			m_plane_y = plane_y.getValue();
			m_grid_spacing = grid_spacing.getValue();
		} else if( shape.getValue() == "sphere") {
			m_default_shape = SPHERE;
			m_radius = radius.getValue();
			m_theta_steps = theta.getValue();
			m_phi_steps   = phi.getValue();
		} else if( shape.getValue() == "cube") {
			m_default_shape = CUBE;
			m_cube_size = cube_size.getValue();
		} else {
			std::cerr << "Not a value shape :" << shape.getValue() << std::endl;
			m_default_shape = PLANE;
			m_plane_x = plane_x.getValue();
			m_plane_y = plane_y.getValue();
			m_grid_spacing = grid_spacing.getValue();
		}

	} catch (TCLAP::ArgException &e) { // catch any exceptions
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
	}
}
