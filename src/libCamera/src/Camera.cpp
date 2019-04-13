#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <Eigen/Geometry>
#include <Camera/Camera.h>

void parseVec3(const std::string& str, float vec[3]) {
	using namespace std;

	istringstream pos(str);

	string val;
	getline(pos, val, ',');
	float x = stof(val);
	getline(pos, val, ',');
	float y = stof(val);
	getline(pos, val);
	float z = stof(val);
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
}

void parseVec2(const std::string& str, float vec[2]) {
	using namespace std;

	istringstream pos(str);

	string val;
	getline(pos, val, ',');
	float x = stof(val);
	getline(pos, val, ',');
	float y = stof(val);
	vec[0] = x;
	vec[1] = y;
}

Camera loadCameraFromFile( const std::string& filename) {
	using namespace std;

	Camera camera;

	bool fl_position, fl_view, fl_up, fl_resolution, fl_fov, fl_f;
	fl_position = fl_view = fl_up = fl_resolution = fl_fov = fl_f = false;

	ifstream file;
	file.exceptions (ifstream::failbit | ifstream::badbit);


	try {
		file.open(filename);
	}
	catch (ifstream::failure e) {
		cout << "ERROR::CAMFILE::NOT_FOUND " << filename << endl;
		cerr << strerror(errno) << endl;
		return camera;
	}

	file.exceptions (ifstream::goodbit);
	string line;
	while ( getline(file, line) ) {
		istringstream is_line(line);
		string key;
		if ( getline(is_line, key, '=') ) {
			string value;
			if ( getline(is_line, value) ) {
				// Handle the line
				if ( key == "position") {
					parseVec3(value, camera.position);
					fl_position = true;
				} else if ( key == "view" ) {
					parseVec3(value, camera.view);
					fl_view = true;
				} else if ( key == "up" ) {
					parseVec3(value, camera.up);
					fl_up = true;
				} else if ( key == "resolution" ) {
					parseVec2(value, camera.resolution);
					fl_resolution = true;
				} else if ( key == "fov" ) {
					parseVec2(value, camera.fov);
					fl_fov = true;
				} else if ( key == "f" ) {
					camera.focalDistance = stof(value);
					fl_f = true;
				} else {
					throw std::domain_error("CAMFILE::UNKNOWN_KEY");
					cerr << "ERROR::CAMFILE::UNKNOWN_KEY " << key << endl;
				}
			}
		}
	}


	if ( !( fl_position && fl_view && fl_up && fl_resolution && fl_fov && fl_f ) ) {
		throw std::domain_error("CAMFILE::MISSING_KEY");
		cerr << "ERROR::CAMFILE::MISSING_KEY" << endl;
	}

	return camera;
}

/*
 * Get the camera matrix
 */
void camera_intrinsics(const Camera& camera, Eigen::Matrix3f& K ) {

	float cx = camera.resolution[0] / 2.0f;
	float cy = camera.resolution[1] / 2.0f;
	float fx = camera.resolution[0] / tan(camera.fov[0] / 2.0f);
	float fy = camera.resolution[1] / tan(camera.fov[1] / 2.0f);
	float skew = 0.0f;

	K << fx,   skew, cx, 
	     0.0f, fy,   cy,
	     0.0f, 0.0f, 1.0f;
}

/**
 * Based on https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
 */
void camera_extrinsics(const Camera& camera, Eigen::Matrix3f& R, Eigen::Vector3f& t ) {
	using namespace Eigen;

	Vector3f position{camera.position[0],camera.position[1],camera.position[2]};
	Vector3f forward = Vector3f{camera.view[0], camera.view[1],camera.view[2]} - position;
	forward.normalize();

	Vector3f up{camera.up[0], camera.up[1], camera.up[2]};
	up.normalize();

	Vector3f s = forward.cross(up);
	s.normalize();
	Vector3f u = s.cross(forward);

	R << s.x(), s.y(), s.z(), 
	     u.x(), u.y(), u.z(),
	     -forward.x(), -forward.y(), -forward.z();

	t = -R * position;
}

void decomposeCamera( const Camera& camera, Eigen::Matrix3f& K, Eigen::Matrix3f& R, Eigen::Vector3f& t ) {
	camera_intrinsics(camera, K );
	camera_extrinsics(camera, R, t );
}


/*
 * Compute the backprojection of a point from X,Y and depth plus camera
 */
Eigen::Vector3f backproject(int pixel_x, int pixel_y, 
						    float depth, 
							const Eigen::Matrix3f& K,	// Camera intrinsics
							const Eigen::Matrix3f& R,	// Rotation matrix (of cam wrt world)
							const Eigen::Vector3f& t  // Location of cam(0,0) wrt world
							) 
{
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

std::ostream& operator<<(std::ostream& os, const Camera& camera) {
	using namespace std;
	os << "pos : " << camera.position[0] << ", " << camera.position[1] << ", " << camera.position[2] << endl;
	os << "vew : " << camera.view[0] << ", " << camera.view[1] << ", " << camera.view[2] << endl;
	os << " up : " << camera.up[0] << ", " << camera.up[1] << ", " << camera.up[2] << endl;
	os << "res : " << camera.resolution[0] << ", " << camera.resolution[1] << endl;
	os << "fov : " << camera.fov[0] << ", " << camera.fov[1] << endl;
	os << "foc : " << camera.focalDistance << endl;
	return os;
}


