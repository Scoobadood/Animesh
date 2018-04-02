class Args {
public:
	 Args( int &argc, char **argv);

	 enum Shape {
	 	PLANE,
	 	CUBE,
	 	SPHERE,
	 	TRIANGLE
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

	 int inline cube_size( ) const { return m_cube_size; }

	 bool inline tracing_enabled() const { return m_tracing_enabled; }

private:
	bool m_should_fix_tangents;

	bool m_should_dump_field;

	int m_num_smoothing_iterations;

	int m_phi_steps;
	int m_theta_steps;

	float m_radius;

	int m_plane_x;
	int m_plane_y;
	float m_grid_spacing;

	int m_cube_size;

	bool m_tracing_enabled;

	Shape m_default_shape;
};