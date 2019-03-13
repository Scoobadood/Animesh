#include <vcg/space/point3.h>
#include <vcg/space/plane3.h>
#include <vcg/space/fitting3.h>
#include <vcg/space/index/kdtree/kdtree.h>
#include <Eigen/Core>
#include <FileUtils/PgmFileParser.h>
#include "depth_image_loader.h"

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
	Vector3f cc  = depth * ray_direction;
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

/*
 * Get the camera matrix
 */
Eigen::Matrix3f camera_intrinsics( ) {
	Eigen::Matrix3f K;
	K << 2029.0f, 0.0f,    320.0f,
	     0.0f,    1522.0f, 240.0f,
	     0.0f,    0.0f,    1.0f;
	return K;
}


/**
 * Compute the normals for each point in the given point cloud.
 * @param all_points The point cloud.
 * @param neighbour_indices The indices of neighbouring points as a vector for each point in the cloud.
 * @param points_with_normal Populated by this method.
 */
void
compute_surface_normals(const std::vector<vcg::Point3f>& 		all_points,
						std::vector<PointWithNormal>&    		points_with_normals,
						std::vector<std::vector<unsigned int>>& neighbour_indices)
{
	using namespace std;
	using namespace Eigen;
	using namespace vcg;

	// Construct KdTree to interrogate nearest neighbours
	ConstDataWrapper<Point3f> wrapped_points{all_points.data(), static_cast<int>(all_points.size()), sizeof( Point3f)};
	KdTree<float> tree(wrapped_points);

	// Extract neighbour indices of points for all points.
	for( auto point : all_points ) {
		vector<unsigned int> this_point_neighbours;
		vector<float> distances;
		float dist = 5.0f;
		tree.doQueryDist(point, dist, this_point_neighbours, distances);
		neighbour_indices.push_back(this_point_neighbours);
	}

	// Now compute normals for each point
	auto point_iter = all_points.begin();
	for( auto point_neighbours : neighbour_indices ) {
		vector<Point3f> neighbour_points;
		for( auto idx : point_neighbours) {
			neighbour_points.push_back(all_points[idx]);
		}

		Plane3f plane;
		FitPlaneToPointSet(neighbour_points, plane);

		Point3f dir = plane.Direction();
		Point3f point = *point_iter++;
		points_with_normals.push_back(PointWithNormal{
			Vector3f{point[0], point[1], point[2]},
			Vector3f{dir[0], dir[1], dir[2]}
		});
	}
}

/**
 * Read the camera data for a given file/frame.
 */
void
read_camera_data(const std::string& file_name,
				 Eigen::Matrix3f& K,
				 Eigen::Matrix3f& R,
				 Eigen::Vector3f& t) {
	// TODO: Replace this with data read from a file.
	K = camera_intrinsics();
	R = Matrix3f::Identity(3,3);
	t << 2.0f, 0.5f, 0.0f;
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
	using namespace vcg;

	points_with_normls.clear();
	neighbour_indices.clear();

	PgmData pgm = read_pgm(file_name);

	read_camera_data(file_name, K, R, t);

	// For all pixels wth a valid depth, generate a 3 point and store
	// to all_points
	vector<Point3f> all_points;
	size_t idx = 0;
	for( int y=0; y<pgm.height; ++y) {
		for( int x = 0; x < pgm.width; ++x ) {
			float depth = pgm.data[idx];
			if( depth != 0 ) {
				Vector3f xyz = backproject(x, y, depth, K, R, t);
				Point3f pt{xyz[0], xyz[1], xyz[2]};
				all_points.push_back(pt);
			}
			++idx;
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

	for( auto file_name : file_names ) {

		vector<PointWithNormal> points_with_normals;
		vector<vector<unsigned int>> neighbour_indices;
		load_depth_image(file_name, points_with_normals, neighbour_indices);
		point_clouds.push_back( points_with_normals );
		neighbours.push_back(neighbour_indices);
	}
}

