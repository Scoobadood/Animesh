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
	Matrix3f kinv;
	kinv << 1.0f / K(0,0), 0.0f,          -K(0,2) / K(0,0),
	        0.0f,          1.0f / K(1,1), -K(1,2) / K(1,1), 
	        0.0f,          0.0f,          1.0f;

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
 * Load depth images from disk and convert to point couds.
 */
std::vector<PointWithNormal>
load_depth_image(const std::string& file_name) {
	using namespace std;
	using namespace Eigen;
	using namespace vcg;

	PgmData pgm = read_pgm(file_name);
	Matrix3f K = Matrix3f::Identity(3,3);
	Matrix3f R = Matrix3f::Identity(3,3);
	Vector3f t = Vector3f::Zero();
	vector<Point3f> all_points;
	size_t idx = 0;
	for( int y=0; y<pgm.height; ++y) {
		for( int x = 0; x < pgm.width; ++x ) {
			float depth = pgm.data[idx];

			Vector3f xyz = backproject(x, y, depth, K, R, t);
			Point3f pt{xyz[0], xyz[1], xyz[2]};
			all_points.push_back(pt);

			++idx;
		}
	}	

	// Construct KdTree to interrogate nearest neighbours
	ConstDataWrapper<Point3f> wrapped_points{all_points.data(), static_cast<int>(all_points.size()), sizeof( Point3f)};
	KdTree<float> tree(wrapped_points);

	vector<PointWithNormal> pointsWithNormals;

	// Given back projected points, fit plane to each point and extract normal
	Plane3f plane;
	vector<Point3f> neighbours;
	for( auto point : all_points ) {
		vector<unsigned int> point_indices;
		vector<float> distances;
		float dist = 0.01f;
		tree.doQueryDist(point, dist, point_indices, distances);
		vector<Point3f> neighbours;
		for( auto idx : point_indices) {
			neighbours.push_back(all_points[idx]);
		}
		FitPlaneToPointSet(neighbours, plane);

		Point3f dir = plane.Direction();
		pointsWithNormals.push_back(PointWithNormal{
			Vector3f{point[0], point[1], point[2]},
			Vector3f{dir[0], dir[1], dir[2]}
		});
		// Take xyz and plane normal and push combined thing into Frame data.
	}
	return pointsWithNormals;
}
