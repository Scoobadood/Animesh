#include <Eigen/Core>
#include <FileUtils/PgmFileParser.h>
#include <FileUtils/FileUtils.h>
#include <Camera/Camera.h>
#include <DepthMap/DepthMap.h>
#include "depth_image_loader.h"

#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>

static int DEBUG_LEVEL = 1;


struct sort_by_distance {
    bool operator ()(std::pair<float, int> const& a, std::pair<float, int> const& b) {
        return a.first < b.first;
    }
};

void 
sort_indices_by_distance( std::vector<unsigned int>& indices, const std::vector<float>& distances ) {
	using namespace std;

	vector<pair<float, int>> orderable;
	for( int i=0; i<indices.size(); ++i ) {
		orderable.push_back(make_pair(distances.at(i), indices.at(i)));
	}
	sort(orderable.begin(), orderable.end(), sort_by_distance());

	// Replace values in indices.
	indices.clear();
	for( int i=0; i<orderable.size(); ++i ) {
		indices.push_back(orderable.at(i).second);
	}
}

void
get_neighbours_for_point_distance(	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree, 
									float distance, 
									const pcl::PointXYZ& point, 
									std::vector<unsigned int>& neighbours) {
	using namespace std;
	using namespace pcl;

	const size_t max_nn = 10;

	vector<float> sq_distances;
	vector<int> neighbour_int;
	sq_distances.reserve(max_nn);
	neighbour_int.reserve(max_nn);

	tree->radiusSearch	(point, distance, neighbour_int, sq_distances, max_nn); // Max 10

	neighbours.clear();
	for( auto i : neighbour_int) {
		neighbours.push_back(i);
	}
}

void
get_neighbours_for_point_count(	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree, 
								unsigned int k, 
								const pcl::PointXYZ& point, 
								std::vector<unsigned int>& neighbours) {
	using namespace std;
	using namespace pcl;

	vector<float> sq_distances;
	vector<int> neighbour_int;
	sq_distances.reserve(k);
	neighbour_int.reserve(k);

	tree->nearestKSearch(point, k, neighbour_int, sq_distances);

	neighbours.clear();
	for( auto i : neighbour_int) {
		neighbours.push_back(i);
	}
}

void
get_neighbours_for_point(	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree, 
							const pcl::PointXYZ& point, 
							std::vector<unsigned int>& neighbours) {
	get_neighbours_for_point_count(tree, 10, point, neighbours);
}

/**
 * Compute the normals for each point in the given point cloud.
 * @param all_points The point cloud.
 * @param neighbour_indices The indices of neighbouring points as a vector for each point in the cloud.
 * @param points_with_normal Populated by this method.
 */
void
compute_surface_normals(const pcl::PointCloud<pcl::PointXYZ>&	all_points,
						std::vector<PointWithNormal>&    		points_with_normals,
						std::vector<std::vector<unsigned int>>& neighbour_indices)
{
	using namespace std;
	using namespace Eigen;
	using namespace pcl;

	// Create the normal estimation class, and pass the input dataset to it
  	NormalEstimation<PointXYZ, Normal> ne;
  	pcl::PointCloud<PointXYZ>::Ptr p = all_points.makeShared();
  	ne.setInputCloud(p);

	// Create an empty kdtree representation, and pass it to the normal estimation object.
	// Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
	search::KdTree<PointXYZ>::Ptr tree (new search::KdTree<PointXYZ> ());
	ne.setSearchMethod (tree);

	// Output datasets
	PointCloud<Normal> cloud_normals;

	// // Use all neighbors in a sphere of radius 3cm
	// ne.setRadiusSearch (5.0);
	ne.setKSearch(25);

	// Compute the features
	ne.compute (cloud_normals);
	auto point_iter = all_points.begin();
	for( auto normal : cloud_normals ) {
		PointXYZ point = *point_iter;
		if( !isnan(normal.normal_x) &&  !isnan(normal.normal_y) && !isnan(normal.normal_z) ) {
			points_with_normals.push_back(PointWithNormal{
				Vector3f{point.x, point.y, point.z},
				Vector3f{normal.normal_x, normal.normal_y, normal.normal_z}
			});
			point_iter++;
		} else {
			cout << "Bad normal (" << normal.normal_x << ", " << normal.normal_y << ", " << normal.normal_z << 
			") << for point at (" << point.x << "," << point.y << "," << point.z << ")" << endl;
		}
	}

	// Get the neighbours of this point too.
	neighbour_indices.clear();
	for( auto point : all_points ) {
		vector<unsigned int> neighbours;
		get_neighbours_for_point(tree, point, neighbours);
		neighbour_indices.push_back(neighbours);
	}
}

/**
 * Read a depth image.
 */
void
read_depth_file(const std::string& 					file_name,
				unsigned int& 						width, 
				unsigned int& 						height,
				std::vector<std::vector<float>>&	depth_data) {
	using namespace std;

	width = 0;
	process_file_by_lines( file_name, 
		[&](const string& text_line){
			using namespace std;
			vector<float> depth_image_row;
			vector<string> tokens = tokenise(text_line);
			if( width == 0 ) {
				width = tokens.size();
			} else {
				if( width != tokens.size() ) {
					string message = "Lines of file must all be the same length";
					throw std::domain_error( message );
				}
			}
			for( auto token : tokens ) {
				float f = stof(token);
				depth_image_row.push_back(f);
			}
			depth_data.push_back(depth_image_row);
	}); 
	height = depth_data.size();
}


/**
 * Load one depth image point cloud.
 * @param file_name The file from which to load - expected to be a PGM file.
 * @param neighbour_indices The indices of neighbouring points as a vector for each point in the cloud.
 * @param points_with_normal Populated by this method.
 */
void
load_depth_image(const std::string& 						file_name,
				 std::vector<PointWithNormal>&    			points_with_normals,
				 std::vector<std::vector<unsigned int>>& 	neighbour_indices)
{
	using namespace std;
	using namespace Eigen;
	using namespace pcl;

	points_with_normals.clear();
	neighbour_indices.clear();

	PointCloud<PointXYZ>	all_points;
	// Each row is a space separated set of floats
	std::vector<std::vector<float>> depth_data;
	unsigned int width;
	unsigned int height;
	read_depth_file(file_name, width, height, depth_data);

	Matrix3f K, R;
	Vector3f t;

	// TODO : Derive this from file_name
	string cam_file_name = "camera.txt";
	Camera camera = loadCameraFromFile(cam_file_name);

	// For all pixels wth a valid depth, generate a 3 point and store
	// to all_points
	for( int y=0; y<height; ++y) {
		for( int x = 0; x < width; ++x ) {
			float depth = depth_data.at(y).at(x);
			if( depth != 0.0f ) {
				Vector3f xyz = backproject(camera, x, y, depth);

				PointXYZ pt{xyz[0], xyz[1], xyz[2]};
				all_points.push_back(pt);
			}
		}
	}	
	compute_surface_normals(all_points, points_with_normals, neighbour_indices);
}
/*
 * Load depth images from disk and convert to point couds.
 * @param file_names The input file names of depth images, assumed to be PGM files.
 * @param point_clouds A vector which is populated by this method. For each frame, for each point, a PointWithNormal.
 * @param neighbours Populated by this method. For each frame, for each point, the neighbouring point indices used to compute the normal.
 */
void
load_depth_images(	const std::vector<std::string>& 						file_names,
					std::vector<std::vector<PointWithNormal>>& 				point_clouds,
					std::vector<std::vector<std::vector<unsigned int>>>&	neighbours)
{
	using namespace std;
	point_clouds.clear();
	neighbours.clear();

	int current_frame_index = 0;
	for( auto file_name : file_names ) {

		vector<PointWithNormal> points_with_normals;
		vector<vector<unsigned int>> neighbour_indices;
		load_depth_image(file_name, points_with_normals, neighbour_indices);
		cout << "dept: frame " << current_frame_index << " has " << points_with_normals.size() << " pixels" << endl;
		++current_frame_index;

		point_clouds.push_back( points_with_normals );
		neighbours.push_back(neighbour_indices);
	}
}



//
//
// 2.5D Handling
//
//

/**
 * Load one depth image point cloud.
 * @param file_name The file from which to load - expected to be a PGM file.
 * @param neighbour_indices The indices of neighbouring points as a vector for each point in the cloud.
 * @param points_with_normal Populated by this method.
 */
void
load_depth_image(const std::string& 						file_name,
				 std::vector<PointWithNormal2_5D>&    		points_with_normals,
				 std::vector<std::vector<unsigned int>>& 	neighbour_indices) {
	using namespace std;
	using namespace Eigen;
	using namespace pcl;

	points_with_normals.clear();
	neighbour_indices.clear();

	// Each row is a space separated set of floats
	DepthMap dm{file_name};
	dm.cull_unreliable_depths(8.0f,3.0f);

//	compute_surface_normals(all_points, points_with_normals, neighbour_indices);
}

/**
 * Read depth images and convert to 2.5D clouds
 * @param file_names The input file names of depth images, assumed to be PGM files.
 * @param point_clouds A vector which is populated by this method. For each frame, for each point, a PointWithNormal.
 * @param neighbours Populated by this method. For each frame, for each point, the neighbouring point indices used to compute the normal.
 */
void 
load_depth_images(const std::vector<std::string>& 						file_names,
				  std::vector<std::vector<PointWithNormal2_5D>>& 		point_clouds,
				  std::vector<std::vector<std::vector<unsigned int>>>&	neighbours) {
	using namespace std;

	// Clear out any residual data
	point_clouds.clear();
	neighbours.clear();

	int current_frame_index = 0;
	for( auto file_name : file_names ) {
		vector<PointWithNormal2_5D> points_with_normals;
		vector<vector<unsigned int>> neighbour_indices;

		// Load a single file
		load_depth_image(file_name, points_with_normals, neighbour_indices);

		++current_frame_index;

		point_clouds.push_back( points_with_normals );
		neighbours.push_back(neighbour_indices);
	}
}
