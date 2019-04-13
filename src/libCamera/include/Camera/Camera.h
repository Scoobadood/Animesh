#include <ostream>
#include <string>
#include <Eigen/Core>

typedef struct Camera {
	float position[3];
	float view[3];
	float up[3];
	float resolution[2];
	float fov[2];
	float focalDistance;
} Camera;

Camera loadCameraFromFile( const std::string& filename);
void decomposeCamera( const Camera& camera, Eigen::Matrix3f& K, Eigen::Matrix3f& R, Eigen::Vector3f& t );
std::ostream& operator<<(std::ostream& os, const Camera& camera);

/*
 * Compute the backprojection of a point from X,Y and depth plus camera
 */
Eigen::Vector3f backproject(int pixel_x, int pixel_y, 
						    float depth, 
							const Eigen::Matrix3f& K,	// Camera intrinsics
							const Eigen::Matrix3f& R,	// Rotation matrix (of cam wrt world)
							const Eigen::Vector3f& t  // Location of cam(0,0) wrt world
							);
