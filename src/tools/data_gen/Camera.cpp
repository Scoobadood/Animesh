#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "Camera.h"

cl_float3 parseVec3(const std::string& str) {
	using namespace std;

	istringstream pos(str);

	string val;
	getline(pos, val, ',');
	float x = stof(val);
	getline(pos, val, ',');
	float y = stof(val);
	getline(pos, val);
	float z = stof(val);
	return cl_float3{x, y, z};
}

cl_float2 parseVec2(const std::string& str) {
	using namespace std;

	istringstream pos(str);

	string val;
	getline(pos, val, ',');
	float x = stof(val);
	getline(pos, val, ',');
	float y = stof(val);
	return cl_float2{x, y};
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
					camera.position = parseVec3(value);
					fl_position = true;
				} else if ( key == "view" ) {
					camera.view = parseVec3(value);
					fl_view = true;
				} else if ( key == "up" ) {
					camera.up = parseVec3(value);
					fl_up = true;
				} else if ( key == "resolution" ) {
					camera.resolution = parseVec2(value);
					fl_resolution = true;
				} else if ( key == "fov" ) {
					camera.fov = parseVec2(value);
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
