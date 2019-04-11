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
