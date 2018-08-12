
#include <string>

class Args {
public:
	 Args( int &argc, char **argv);

	 enum Shape {
	 	PLANE,
	 	CUBE,
	 	SPHERE,
	 	CIRCLE,
	 	POLYNOMIAL
	 };

	 /** 
	  * @return true if the --fix or -f flag is present
	  */
	 bool inline should_fix_tangents() const { return m_should_fix_tangents; }

	 /** 
	  * @return true if the --dump or -d flag is present
	  */
	 bool inline should_dump_field() const { return m_should_dump_field; }

	 /**
	  * @return  The number of smoothing iterations to perform, specified by --iter
	  */
	 int inline num_iterations() const { return m_num_smoothing_iterations; }

	 Shape inline default_shape() const { return m_default_shape; }

	 int inline plane_x() const { return m_plane_x; }
	 int inline plane_y() const { return m_plane_y; }
	 float inline grid_spacing() const { return m_grid_spacing; }

	 float inline radius() const { return m_radius; }

	 int inline theta_steps() const { return m_theta_steps; }
	 int inline phi_steps() const { return m_phi_steps; }

	 int inline cube_x( ) const { return m_cube_x; }
	 int inline cube_y( ) const { return m_cube_y; }
	 int inline cube_z( ) const { return m_cube_z; }
	 int inline k( ) const { return m_k; }

	 bool inline tracing_enabled() const { return m_tracing_enabled; }

	 bool inline load_from_pointcloud() const { return m_load_from_pointcloud; }
	 std::string inline file_name() const { return m_file_name; }

private:
	bool 			m_should_fix_tangents;

	bool 			m_should_dump_field;

	int 			m_num_smoothing_iterations;

	int 			m_phi_steps;
	int 			m_theta_steps;

	float 			m_radius;

	int 			m_plane_x;
	int 			m_plane_y;
	float 			m_grid_spacing;

	int 			m_cube_x;
	int 			m_cube_y;
	int 			m_cube_z;

	int 			m_k;

	bool 			m_tracing_enabled;

	bool 			m_load_from_pointcloud;
	std::string		m_file_name;

	Shape 			m_default_shape;
};