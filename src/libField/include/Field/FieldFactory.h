/**
 * Makes Fields on demand
 * Part of Animesh
 * Author Dave Durbin david.j.durbin@uts.edu.au
 */

#pragma once

#include <Field/Field.h>

namespace animesh {

class FieldFactory {
public:
	/* ******************************************************************************************
	 * *
	 * *  Construct prefab fields
	 * *
	 * ******************************************************************************************/
	static Field * planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k );
	static Field * cubic_field( int cube_x, int cube_y, int cube_z, float scale, int k);
	static Field * polynomial_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k);
	static Field * spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, int k);
	static Field * circular_field( float radius, int k );

};

/**
* Load an obj file into a point cloud
*/
pcl::PointCloud<pcl::PointNormal>::Ptr load_pointcloud_from_obj( const std::string& file_name );

/**
 * Construct a field from an OBJ file
 */
Field * load_field_from_obj_file( const std::string& file_name, int k = 5, float with_scaling = 1.0f, bool trace = false );
}