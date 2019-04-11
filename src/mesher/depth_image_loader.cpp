#include <Eigen/Core>
#include <FileUtils/PgmFileParser.h>
#include <FileUtils/FileUtils.h>
#include <Camera/Camera.h>
#include "depth_image_loader.h"

#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>

static int DEBUG_LEVEL = 1;

/*
 * Compute the backprojection of a point from X,Y and depth plus camera
 */
Eigen::Vector3f backproject(int pixel_x, int pixel_y, float depth, 
							Eigen::Matrix3f K,	// Camera intrinsics
							Eigen::Matrix3f R,	// Rotation matrix (of cam wrt world)
							Eigen::Vector3f t  // Location of cam(0,0) wrt world
							) {
	using namespace Eigen;

	// Image Coord = External Camera Matrix * Internal * Projection 
	// We assume that camera is this matrix

	// Make back proj matrix
	Matrix3f kinv = K.inverse();

	Vector3f point{ pixel_x, pixel_y, 1.0f };
	Vector3f ray_direction = kinv * point;
	// TODO: Find a better way of handling this scale favtor which was injected by cheat data contruction.
	Vector3f cc  = (depth * ray_direction) / 255.0f;
	Vector4f camera_coord  = Vector4f{cc(0), cc(1), cc(2), 1.0f};

	Matrix4f extrinsic;
	Matrix3f RcT = R.transpose();
	Vector3f tc   = -R * t;

	extrinsic << RcT(0,0), RcT(1,0), RcT(2,0), tc(0),
				  RcT(0,1), RcT(1,1), RcT(2,1), tc(1),
				  RcT(0,2), RcT(1,2), RcT(2,2), tc(2),
				  0.0f, 0.0f, 0.0f, 1.0f;

	Vector4f world_coord = extrinsic.inverse() * camera_coord;
	float scale = 1.0f / world_coord[3];
	return Vector3f{ world_coord[0], world_coord[1], world_coord[2]} * scale;
}

/**
 * Read the camera data for a given file/frame.
 */
void
read_camera_data(const std::string& file_name,
				 Eigen::Matrix3f& K,
				 Eigen::Matrix3f& R,
				 Eigen::Vector3f& t) {
	Camera camera = loadCameraFromFile(file_name);
	decomposeCamera(camera, K, R, t);
}


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

	// Record start time
	auto start = std::chrono::high_resolution_clock::now();

	// Each row is a space separated set of floats
	std::vector<std::vector<float>> depth_data;
	unsigned int width = 0;
	process_file_by_lines( file_name, 
		[&](const string& text_line){
			using namespace std;
			std::vector<float> depth_image_row;
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
	unsigned int height = depth_data.size();

	Matrix3f K, R;
	Vector3f t;
	auto finish_read = std::chrono::high_resolution_clock::now();

	// TODO : Derive this from file_name
	string cam_file_name = "camera.txt";
	read_camera_data(cam_file_name, K, R, t);

	// For all pixels wth a valid depth, generate a 3 point and store
	// to all_points
	pcl::PointCloud<pcl::PointXYZ> all_points;

	for( int y=0; y<height; ++y) {
		for( int x = 0; x < width; ++x ) {
			float depth = depth_data.at(y).at(x);
			if( depth != 0.0f ) {
				Vector3f xyz = backproject(x, y, depth, K, R, t);

				PointXYZ pt{xyz[0], xyz[1], xyz[2]};
				all_points.push_back(pt);
			}
		}
	}	
	auto finish_back_project = std::chrono::high_resolution_clock::now();
	compute_surface_normals(all_points, points_with_normals, neighbour_indices);

	// Record end time
	auto finish_normals = std::chrono::high_resolution_clock::now();
	cout << "load_depth_image" << endl
	     << "       file read : " << (chrono::duration_cast<chrono::milliseconds>(finish_read - start)).count() << "ms" << endl
	     << "    back project : " << (chrono::duration_cast<chrono::milliseconds>(finish_back_project - finish_read)).count() << "ms" << endl
	     << " compute normals : " << (chrono::duration_cast<chrono::milliseconds>(finish_normals - finish_back_project)).count() << "ms" << endl;
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

