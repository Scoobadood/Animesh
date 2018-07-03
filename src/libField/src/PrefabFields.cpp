
#include <Field/Field.h>
#include <Eigen/Core>
#include <Graph/ExplicitGraphBuilder.h>
#include <Graph/GridGraphBuilder.h>
#include <Graph/NearestNeighbourGraphBuilder.h>
#include <vector>
#include <map>


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

	return new Field( cloud, k, false );
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

	return new Field(cloud, k, false );
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

    pcl::PointCloud<pcl::PointNormal>::Ptr cloud (new pcl::PointCloud<pcl::PointNormal>);

	float maxTheta = M_PI;
	float maxPhi = M_PI / 2.0f;
	float deltaTheta = maxTheta / theta_steps;
	float deltaPhi   = maxPhi / phi_steps;

	std::vector<Element> elements;

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
			point.x = radius * point.x;
			point.y = radius * point.y;
			point.z = radius * point.z;
			cloud->push_back( point );
		}
	}

	return new Field( cloud, k, false );
}


Field * Field::circular_field( float radius, int k ) {
	using namespace Eigen;
	std::vector<Element> elements;

	for ( int i=0; i<2; ++i) {
		float theta = 0.0f;
		for( int i = 0; i<40; i++ ) {
			float x = radius * std::cos( theta );
			float y = radius * std::sin( theta );

			Vector3f location{ x, y, 0.0f};
			Vector3f normal{ 0, 0, 1 };
			Element e{ location, normal };
			elements.push_back( e );

			theta += (2 * M_PI / 40 );
		}
		radius -= 1.0f;
	}

	NearestNeighbourGraphBuilder<void*> * gb = new NearestNeighbourGraphBuilder<void*>( k );
	Field * field = new Field( gb, elements );
	delete gb;

	return field;
}

Field * Field::cubic_field( std::size_t cube_size, float scale, int k) {
	using namespace Eigen;

	float minVal = (0.5f - ( cube_size / 2.0f )) * scale;
	float maxVal = (( cube_size / 2.0f ) - 0.5f) * scale;

	std::vector<Element> elements;

	Vector3f normalZ{ 0, 0, 1 };
	for( float x = minVal; x <= maxVal; x +=  scale) {
		for( float y = minVal; y <= maxVal; y+= scale ) {

			Vector3f location { x, y, minVal - (scale / 2.0f) };
			Element e{ location, -normalZ };
			elements.push_back( e );

			Vector3f location2 { x, y, maxVal + (scale / 2.0f) };
			Element e2{ location2, normalZ };
			elements.push_back( e2 );
		}
	}
	Vector3f normalY{ 0, 1, 0 };
	for( float x = minVal; x <= maxVal; x +=  scale) {
		for( float z = minVal; z <= maxVal; z+= scale ) {
			Vector3f location { x, minVal - (scale / 2.0f), z };
			Element e{ location, -normalY };
			elements.push_back( e );

			Vector3f location2 { x, maxVal + (scale / 2.0f), z };
			Element e2 {location2, normalY };
			elements.push_back( e2 );
		}
	}
	Vector3f normalX{ 1, 0, 0 };
	for( float z = minVal; z <= maxVal; z +=  scale) {
		for( float y = minVal; y <= maxVal; y+= scale ) {
			Vector3f location { minVal - (scale / 2.0f), y, z };
			Element e{ location, -normalX };
			elements.push_back( e );

			Vector3f location2 { maxVal + (scale / 2.0f), y, z };
			Element e2{location2, normalX };
			elements.push_back( e2 );
		}
	}

	NearestNeighbourGraphBuilder<void*> * gb = new NearestNeighbourGraphBuilder<void*>( 8 );
	Field * field = new Field( gb, elements );
	delete gb;

	return field;
}
