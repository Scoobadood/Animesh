
#include <Field/Field.h>
#include <Eigen/Core>
#include <vector>
#include <map>
#include <iostream>

namespace animesh {


/**
 * Construct a curved field centred at (0,0,0). THe field is defined by z=1-\frac{1}{10}\left( x^{2}+y^{2} \right)
 * with grid neighbourhod
 * @param dim_x The number of points in the X plane
 * @param dim_y The number of points in the Y plane
 * @param grid_spacing The space between grid points
 * @param k The number of nearest neighbours to consider
 */
Field * Field::polynomial_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k) {
	using namespace Eigen;

	std::cout << "Plane x:" << dim_x << ", y:" << dim_y << ", sp:"<<grid_spacing << std::endl;

    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);

	int minx = -dim_x/2, maxx = minx+dim_x-1;
	int miny = -dim_y/2, maxy = miny+dim_y-1;
	float divisor = (10.0f * grid_spacing * grid_spacing);
	for( int y = miny; y <= maxy; y++ ) {
		for( int x = minx; x <= maxx; x++ ) {

			pcl::PointNormal point;
			point.x = x * grid_spacing;
			point.y = y * grid_spacing;
			point.z = 1 - (point.x*point.x+point.y*point.y)/divisor;

			// Normal is (dz/dx, dz/dy, -1)
			Eigen::Vector3f normal{ (2 * point.x) / divisor, (2 * point.y) / divisor, 1.0f };
			normal.normalize();
			point.normal_x = normal[0];
			point.normal_y = normal[1];
			point.normal_z = normal[2];
			cloud->push_back( point );
		}
	}

	return new Field( cloud, k, true );
}

/**
 * Construct a planar field centred at (0,0,0)
 * @param dim_x The number of points in the X plane
 * @param dim_y The number of points in the Y plane
 * @param grid_spacing The space between grid points
 * @param k The number of nearest neighbours to consider
 */
Field * Field::planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, int k ) {
	using namespace Eigen;

	std::cout << "Plane x:" << dim_x << ", y:" << dim_y << ", sp:"<<grid_spacing << std::endl;

    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);

	int minx = -dim_x/2, maxx = minx+dim_x-1;
	int miny = -dim_y/2, maxy = miny+dim_y-1;
	for( int y = miny; y <= maxy; y++ ) {
		for( int x = minx; x <= maxx; x++ ) {

			pcl::PointNormal point;
			point.normal_x = 0.0f;
			point.normal_y = 0.0f;
			point.normal_z = 1.0f;
			point.x = x * grid_spacing;
			point.y = y * grid_spacing;
			point.z = 0.0f;
			cloud->push_back( point );
		}
	}

	return new Field(cloud, k, true );
}

/**
 * Construct a spherical field centred at (0,0,0)
 * @param radius The radius of the sphere to be constructed
 * @param theta_steps The number of steps around the sphere (in XZ plane)
 * @param phi_steps The number of steps in the Y direction
 * @param k The number of nearest neighbours to consider
 */
Field * Field::spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, int k ) {
	using namespace Eigen;

	std::cout << "Sphere ra:" << radius << ", ts:" << theta_steps << ", ps:" << phi_steps << std::endl;

    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);

	float maxTheta = M_PI;
	float maxPhi = M_PI;
	float deltaTheta = maxTheta / theta_steps;
	float deltaPhi   = maxPhi / phi_steps;

	// Add top and bottom points
	pcl::PointNormal point;
	point.normal_y = 1.0f;
	point.y = radius;
	cloud->push_back( point );

	point.normal_y = -1.0f;
	point.y = -radius;
	cloud->push_back( point );

	for( float theta = 0.0f; theta < maxTheta; theta +=  deltaTheta) {
		for( float phi = maxPhi-deltaPhi; phi>-maxPhi; phi -= deltaPhi ) {

			pcl::PointNormal point;
			point.normal_x = sin(phi) * cos(theta);
			point.normal_y = cos( phi );
			point.normal_z = sin(phi) * sin(theta);
			point.x = radius * point.normal_x;
			point.y = radius * point.normal_y;
			point.z = radius * point.normal_z;
			cloud->push_back( point );
		}
	}

	return new Field( cloud, k, true );
}


Field * Field::circular_field( float radius, int k ) {
	using namespace Eigen;

    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);

	for ( int i=0; i<2; ++i) {
		float theta = 0.0f;
		for( int i = 0; i<40; i++ ) {
			float x = radius * std::cos( theta );
			float y = radius * std::sin( theta );

			pcl::PointNormal point;
			point.normal_x = 0.0f;
			point.normal_y = 0.0f;
			point.normal_z = 1.0f;
			point.x = x;
			point.y = y;
			point.z = 0.0f;
			cloud->push_back( point );

			theta += (2 * M_PI / 40 );
		}
		radius -= 1.0f;
	}

	return new Field( cloud, k, false );
}

Field * Field::cubic_field( int cube_x, int cube_y, int cube_z, float scale, int k) {
	using namespace Eigen;

	std::cout << "Cube x:" << cube_x << ", y:" << cube_y << ", z:" << cube_z << ", sp:"<<scale << std::endl;


    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);

	float minXVal = (0.5f - ( cube_x / 2.0f )) * scale;
	float maxXVal = (( cube_x / 2.0f ) - 0.5f) * scale;
	float minYVal = (0.5f - ( cube_y / 2.0f )) * scale;
	float maxYVal = (( cube_y / 2.0f ) - 0.5f) * scale;
	float minZVal = (0.5f - ( cube_z / 2.0f )) * scale;
	float maxZVal = (( cube_z / 2.0f ) - 0.5f) * scale;


	pcl::PointNormal point;
	point.normal_x = 0.0f;
	point.normal_y = 0.0f;
	point.normal_z = -1.0f;
	for( float x = minXVal; x <= maxXVal; x +=  scale) {
		for( float y = minYVal; y <= maxYVal; y+= scale ) {

			point.x = x;
			point.y = y;
			point.z = minZVal - (scale / 2.0f);
			cloud->push_back( point );

			point.z = maxZVal + (scale / 2.0f);
			point.normal_z = 1.0f;
			cloud->push_back( point );
		}
	}

	point.normal_x = 0.0f;
	point.normal_y =-1.0f;
	point.normal_z = 0.0f;
	for( float x = minXVal; x <= maxXVal; x +=  scale) {
		for( float z = minZVal; z <= maxZVal; z+= scale ) {
			point.x = x;
			point.y = minYVal - (scale / 2.0f);
			point.z = z;
			cloud->push_back( point );

			point.y = maxYVal + (scale / 2.0f);
			point.normal_y = 1.0f;
			cloud->push_back( point );
		}
	}

	point.normal_x =-1.0f;
	point.normal_y = 0.0f;
	point.normal_z = 0.0f;
	for( float z = minZVal; z <= maxZVal; z +=  scale) {
		for( float y = minYVal; y <= maxYVal; y+= scale ) {
			point.x = minXVal - (scale / 2.0f);
			point.y = y;
			point.z = z;
			cloud->push_back( point );

			point.x = maxXVal + (scale / 2.0f);
			point.normal_x = 1.0f;
			cloud->push_back( point );
		}
	}

	return new Field( cloud, k, true );
}

}