#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

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
					cerr << "ERROR::CAMFILE::UNKNOWN_KEY " << key << endl;
				}
			}
		}
	}


	if ( !( fl_position && fl_view && fl_up && fl_resolution && fl_fov && fl_f ) ) {
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
