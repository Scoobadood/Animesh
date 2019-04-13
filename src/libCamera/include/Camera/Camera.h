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

Eigen::Vector3f backproject(const Camera& camera,int pixel_x, int pixel_y, float depth);
