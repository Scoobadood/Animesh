
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
 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
 */
Field * Field::polynomial_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, bool make_fixed ) {
	using namespace Eigen;

	std::vector<Element> elements;
	int minx = -dim_x/2, maxx = minx+dim_x-1;
	int miny = -dim_y/2, maxy = miny+dim_y-1;
	for( int y = miny; y <= maxy; y++ ) {
		for( int x = minx; x <= maxx; x++ ) {
			float xc = x * grid_spacing;
			float yc = y * grid_spacing;
			float divisor = (10.0f * grid_spacing * grid_spacing);
			float zc = 1 - (xc*xc+yc*yc)/divisor;

			Vector3f location { xc, yc, zc };

			// Normals is (dz/dx, dz/dy, -1)
			Vector3f normal{ (2 * xc) / divisor, (2 * yc) / divisor, 1.0f };
			normal.normalize();
			Element e{location, normal };
			elements.push_back( e );
		}
	}

	// Explicitly set neighbours
	std::map<int,std::vector<int>> adjacency_map{};
	int idx = 0;
	for( int y = miny; y <= maxy; y++ ) {
		for( int x = minx; x <= maxx; x++ ) {
			std::vector<int> neighbours;
			if( x > minx ) neighbours.push_back( y * dim_x + x - 1 );
			if( x < maxx ) neighbours.push_back( y * dim_x + x + 1 );
			if( y > miny ) neighbours.push_back( y * dim_x - dim_x + x );
			if( y < maxy ) neighbours.push_back( y * dim_x + dim_x + x );
			adjacency_map[idx] = neighbours;
		}
	}

	ExplicitGraphBuilder<void*> * egb = new ExplicitGraphBuilder<void*>( adjacency_map );
	Field * field = new Field( egb, elements );
	delete egb;

	return field;		
}

/**
 * Construct a planar field centred at (0,0,0)
 * @param dim_x The number of points in the X plane
 * @param dim_y The number of points in the Y plane
 * @param grid_spacing The space between grid points
 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
 */
Field * Field::planar_field( std::size_t dim_x, std::size_t dim_y, float grid_spacing, bool make_fixed ) {
	using namespace Eigen;

	std::vector<Element> elements;
	for( int y = 0; y<dim_y; y++ ) {
		for( int x = 0; x<dim_x; x++ ) {
			Vector3f location { x * grid_spacing, y * grid_spacing, 0.0f };
			Vector3f normal{ 0.0f, 0.0f, 1.0f };
			Element e{location, normal };
			elements.push_back( e );
		}
	}

	GridGraphBuilder<void*> * ggb = new GridGraphBuilder<void*>( grid_spacing );
	Field * field = new Field( ggb, elements );
	delete ggb;

	// Make a set of planar tangents
	if( make_fixed ) {
		std::vector< const Eigen::Vector3f > good_tangents;
		for( std::size_t i=0; i< field->size(); ++i ) {
			float noise = random( );
			good_tangents.push_back( Eigen::Vector3f{ 0.0f, 1.0f, 0.0f } );
		}

		field->set_tangents( good_tangents );
	}

	return field;
}

/**
 * Construct a spherical field centred at (0,0,0)
 * @param radius The radius of the sphere to be constructed
 * @param theta_steps The number of steps around the sphere (in XZ plane)
 * @param phi_steps The number of steps in the Y direction
 * @param make_fixed If true, set the field tangents to the lowest energy/solved position
 */
Field * Field::spherical_field( float radius, std::size_t theta_steps, std::size_t phi_steps, int k, bool make_fixed ) {
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


Field * Field::circular_field( float radius, int k, bool make_fixed ) {
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

	// Make a set of planar tangents
	if( make_fixed ) {
		std::vector< const Eigen::Vector3f > good_tangents;
		for( std::size_t i=0; i< field->size(); ++i ) {
			good_tangents.push_back( Eigen::Vector3f{ 0.0f, 1.0f, 0.0f } );
		}

		field->set_tangents( good_tangents );
	}

	return field;
}

Field * Field::cubic_field( std::size_t cube_size, float scale, bool make_fixed) {
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


	if( make_fixed ) {
		for( auto node_iter = field->m_graph->nodes().begin();
			      node_iter != field->m_graph->nodes().end();
			      ++node_iter ) {

			FieldElement *fe = (*node_iter)->data();

			if( fabsf(fe->m_tangent[0]) < EPSILON && 
				fabsf(fe->m_tangent[1]) < EPSILON && 
				fabsf(fe->m_tangent[2]) < EPSILON ) {
				throw std::invalid_argument( "Found zero tangent for fe");
			}

			if( fe->m_normal[0] < -EPSILON || fe->m_normal[0] > EPSILON || fe->m_normal[2] < -EPSILON || fe->m_normal[2] > EPSILON ) {
				fe->m_tangent = Vector3f{ 0.0f, 1.0f, 0.0f };
			} else if ( fe->m_normal[1] < -EPSILON || fe->m_normal[1] > EPSILON ) {
				fe->m_tangent = Vector3f{ 0.0f, 0.0f, 1.0f };
			} else {
				throw std::invalid_argument( "Unexpected normal in cubic_field");
			}
		}
	}

	return field;
}
