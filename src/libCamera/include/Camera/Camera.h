#include <string>

typedef struct Camera {
	float position[3];
	float view[3];
	float up[3];
	float resolution[2];
	float fov[2];
	float focalDistance;
} Camera;

Camera loadCameraFromFile( const std::string& filename);
