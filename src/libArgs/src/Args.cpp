#include <Args/Args.h>
#include <tclap/CmdLine.h>
#include <tclap/ArgException.h>

const int DEFAULT_SMOOTH_ITERATIONS = 50;
const int DIM_X = 20;
const int DIM_Y = 20;
const float GRID_SPACING = 5.0f;
const float RADIUS = 10.0f;
const int SPHERE_THETA_STEPS = 30;
const int SPHERE_PHI_STEPS = 15;
const int CUBE_SIZE = 3;

std::vector<std::string> split( const std::string& str, char token )  {
	std::vector<std::string> strings;

	std::istringstream f(str);
    std::string s;    
	while (getline(f, s, token)) {		
		 strings.push_back(s);
	}
	return strings;
}


class Prefab {

private:
	std::string   	m_name;
	float			m_grid_spacing;
	int 			m_dim1;
	int 			m_dim2;
	int 			m_dim3;

protected:
	Prefab( const std::string& name, float grid_spacing, int dim1, int dim2, int dim3) : m_name{ name } {
		m_grid_spacing = grid_spacing;
		m_dim1 = dim1;
		m_dim2 = dim2;
		m_dim3 = dim3;
	}

	Prefab( const std::string& name, float grid_spacing, int dim1, int dim2) : m_name{ name } {
		m_grid_spacing = grid_spacing;
		m_dim1 = dim1;
		m_dim2 = dim2;
	}

	Prefab( const std::string& name, float grid_spacing, int dim1) : m_name{ name } {
		m_grid_spacing = grid_spacing;
		m_dim1 = dim1;
	}

public:
	std::string name() const {
		return m_name;
	}

	float radius( ) const {
		if( m_name == "sphere" || m_name == "circle" ) {
			return m_grid_spacing;
		}
		throw std::domain_error( "Radius is only valid for circle or sphere");
	}

	float grid_spacing( ) const {
		if( m_name != "sphere" || m_name != "circle" ) {
			return m_grid_spacing;
		}
		throw std::domain_error( "Grid spacing is not valid for circle or sphere");
	}

	int theta_steps( ) const {
		if( m_name == "sphere" || m_name == "circle" ) {
			return m_dim1;
		}
		throw std::domain_error( "Theta steps is only valid for circle or sphere");
	}

	int phi_steps( ) const {
		if( m_name == "sphere" ) {
			return m_dim2;
		}
		throw std::domain_error( "Phi steps is only valid for sphere");
	}

	int size_x( ) const {
		if( m_name == "plane" || m_name == "poly" || m_name == "cube" ) {
			return m_dim1;
		}
		throw std::domain_error( "size_x is only valid for poly, plane or cube");
	}

	int size_y( ) const {
		if( m_name == "plane" || m_name == "poly" || m_name == "cube" ) {
			return m_dim2;
		}
		throw std::domain_error( "size_y is only valid for poly, plane or cube");
	}

	int size_z( ) const {
		if( m_name == "cube" ) {
			return m_dim3;
		}
		throw std::domain_error( "size_z is only valid for cube");
	}

	static Prefab from_string( const std::string& value ) {
		using namespace TCLAP;

		// Break the string into chunks at commas
		std::vector<std::string> chunks = split( value, ',');
		if( chunks[0] == "cube") {
			if( chunks.size() != 5 )
				throw ArgException( "Cube args should be grid_size, dim_x, dim_y, dim_z" );
			return cube_prefab( chunks );
		} 
		else if( chunks[0] == "sphere") {
			if( chunks.size() != 4 )
				throw ArgException( "Sphere args should be radius, theta steps, phi steps" );
			return sphere_prefab( chunks );
		}
		else if( chunks[0] == "poly") {
			if( chunks.size() != 4 )
				throw ArgException( "Poly args should be grid spacing, x dim, y dim" );
			return poly_prefab( chunks );
		}
		else if( chunks[0] == "plane") {
			if( chunks.size() != 4 )
				throw ArgException( "Plane args should be grid spacing, x dim, y dim" );
			return plane_prefab( chunks );
		}
		else if( chunks[0] == "circle") {
			if( chunks.size() != 3 )
				throw ArgException( "Circle args should be radius, num_steps" );
			return circle_prefab( chunks );
		}
		else 
			throw std::invalid_argument("Unknown prefab type " + chunks[0]);
	}

	static Prefab sphere_prefab( const std::vector<std::string>& chunks ) {
		// Expect radius, theta_step, phi_step and K
		float radius 		= std::stof(chunks[1]);
		int   theta_steps   = std::stoi(chunks[2]);
		int   phi_steps     = std::stoi(chunks[3]);
		return Prefab{ "sphere", radius, theta_steps, phi_steps};
	}

	static Prefab cube_prefab( const std::vector<std::string>& chunks ) {
		// Expect grid_spacing, x dim, y dim, z dim
		float grid_spacing 		= std::stof(chunks[1]);
		int   x_dim   			= std::stoi(chunks[2]);
		int   y_dim			    = std::stoi(chunks[3]);
		int   z_dim 		    = std::stoi(chunks[4]);
		return Prefab{ "cube", grid_spacing, x_dim, y_dim, z_dim};
	}

	static Prefab plane_prefab( const std::vector<std::string>& chunks ) {
		// Expect grid_spacing, x dim, y dim, z dim
		float grid_spacing 		= std::stof(chunks[1]);
		int   x_dim   			= std::stoi(chunks[2]);
		int   y_dim			    = std::stoi(chunks[3]);
		return Prefab{ "plane", grid_spacing, x_dim, y_dim};
	}

	static Prefab poly_prefab( const std::vector<std::string>& chunks ) {
		// Expect grid_spacing, x dim, y dim, z dim
		float grid_spacing 		= std::stof(chunks[1]);
		int   x_dim   			= std::stoi(chunks[2]);
		int   y_dim			    = std::stoi(chunks[3]);
		return Prefab{ "poly", grid_spacing, x_dim, y_dim};
	}

	static Prefab circle_prefab( const std::vector<std::string>& chunks ) {
		// Expect grid_spacing, x dim, y dim, z dim
		float radius 		= std::stof(chunks[1]);
		int   num_steps		= std::stoi(chunks[2]);
		return Prefab{ "circle", radius, num_steps};
	}
};

std::istream& operator>> (std::istream& is, Prefab& p) {  
	std::string s;
	is>> s;  
	p = Prefab::from_string(s);
	return is;  
}


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
		ValueArg<int> iterations("i","iter","Number of smoothing iterations to run per click",false, DEFAULT_SMOOTH_ITERATIONS, "num_iters", cmd);

		// Shapes are all strings and must be one of 
		// Cube,dimx,dimy,dimz,spacing
		// Plane,dimx,dimy,spacing
		// Polynomial,dimx,dimy,spacing
		// Sphere,radius,theta_steps,phi_steps
		// Circle,radius,theta_steps
		std::vector<std::string> plane = {"poly", "1.0", "10", "10"};
		Prefab simple_plane = Prefab::plane_prefab( plane );
		ValueArg<Prefab> prefab("p","prefab","Use a prefab field shape. plane, cube,polynomial, circle or sphere", true, simple_plane, "prefab");
		ValueArg<std::string> file_name("n","file","OBJ File name.",true, "", "filename");
        cmd.xorAdd( prefab, file_name );

		ValueArg<int> k("k","knear","Specify k nearest neighbours to use when building graph", true, 5, "num", cmd);

		SwitchArg make_fixed("f","fix","Make initial field tangents fixed", cmd, false);
		SwitchArg dump_field("d","dump","Dump the field in it's initial state", cmd, false);
		SwitchArg tracing_enabled("v","verbose","Output lots of diagnostics", cmd, false);

		// Scale - byy which to multiply coords
		ValueArg<float> scale("s","scale","Multiplier for point coordinates",false, 1.0f, "scale", cmd);


		// Parse the argv array.
		cmd.parse( argc, argv );

		m_num_smoothing_iterations = iterations.getValue();
		m_should_fix_tangents      = make_fixed.getValue();
		m_should_dump_field        = dump_field.getValue();
		m_tracing_enabled          = tracing_enabled.getValue();
		m_k						   = k.getValue();
		m_scale					   = scale.getValue();


		if( file_name.isSet() ) {
			m_file_name = file_name.getValue();
			m_load_from_pointcloud = true;
		} else if ( prefab.isSet() ) {
			if( prefab.getValue().name() == "plane" || prefab.getValue().name() == "poly" ){
				m_default_shape = PLANE;
				m_plane_x = prefab.getValue().size_x();
				m_plane_y = prefab.getValue().size_y();
				m_grid_spacing = prefab.getValue().grid_spacing();
				if( prefab.getValue().name() == "poly" ) {
					m_default_shape = POLYNOMIAL;
				}
			} else if( prefab.getValue().name() == "sphere") {
				m_default_shape = SPHERE;
				m_radius = prefab.getValue().radius();
				m_theta_steps = prefab.getValue().theta_steps();
				m_phi_steps   = prefab.getValue().phi_steps( );
			} else if( prefab.getValue().name() == "cube") {
				m_default_shape = CUBE;
				m_cube_x = prefab.getValue().size_x();
				m_cube_y = prefab.getValue().size_y();
				m_cube_z = prefab.getValue().size_z();
			} else if( prefab.getValue().name() == "circle") {
				m_default_shape = CIRCLE;
				m_radius = prefab.getValue().radius( );
				m_theta_steps = prefab.getValue().theta_steps( );
			} else {
				throw("Very bad things...");
			}
		} else {
			// No prefab or file: Shold never get here because TCLAP should have handled it
			throw("Very bad things...");
		}
	} catch (TCLAP::ArgException &e) { // catch any exceptions
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
	}
}
