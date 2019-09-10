#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>
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

	R << s.x(), u.x(), -forward.x(),
		 s.y(), u.y(), -forward.y(),
		 s.z(), u.z(), -forward.z();

	t = -R * position;
}

void decomposeCamera( const Camera& camera, Eigen::Matrix3f& K, Eigen::Matrix3f& R, Eigen::Vector3f& t ) {
	camera_intrinsics(camera, K );
	camera_extrinsics(camera, R, t );
	std::cout << "Cam R " << R << std::endl;
	std::cout << "Cam t " << t << std::endl;
}



/**
 * Construct an eye coordinate system
 * Input: camera position, center of interest, view-up vector
 * Returns: new origin and three basis vectors
 *
 *               /|
 *              / |
 *             /  |
 *            /   |
 *           / ^  |
 *          / v|  |
 *          |  |-----> n
 *          | /  /
 *          |Lu /
 *          |  /
 *          | /
 *          |/
 *
 */
void construct_camera_coordinate_system(const Camera& camera, 
										Eigen::Vector3f&  cam_origin, 
										Eigen::Vector3f& n, 
										Eigen::Vector3f& u,
										Eigen::Vector3f& v)
{
	using namespace Eigen;

	// n is normal to image plane, points in opposite direction of view point
	Vector3f N{ camera.position[0] - camera.view[0], 
				camera.position[1] - camera.view[1],
				camera.position[2] - camera.view[2]};
	n = N.normalized();

	// u is a vector that is perpendicular to the plane spanned by
	// N and view up vector (cam->up)
	Vector3f U = Vector3f{camera.up[0], camera.up[1], camera.up[2]}.cross(n);
	u = U.normalized();

	// v is a vector perpendicular to N and U
	v = n.cross(u);

	// origin is cam centre
	cam_origin = Vector3f{camera.position[0], camera.position[1], camera.position[2]};
}


void construct_image_plane_origin(const Eigen::Vector2f& fov, 
								  const float focal_length,
								  const Eigen::Vector3f&  camera_origin, 
								  const Eigen::Vector3f& n, 
								  const Eigen::Vector3f& u, 
								  const Eigen::Vector3f& v, 
								  Eigen::Vector3f& image_plane_origin,
								  Eigen::Vector2f& image_plane_dimensions)
{
	using namespace Eigen;

	float image_plane_height = tan(fov.y() * 0.5f * (M_PI / 180)) * 2.0f * focal_length;
	float image_plane_width  = tan(fov.x() * 0.5f * (M_PI / 180)) * 2.0f * focal_length;

	Vector3f image_plane_centre = camera_origin - ( n * focal_length );
	image_plane_origin = image_plane_centre - (u * image_plane_width * 0.5f) - (v * image_plane_height * 0.5f);
	image_plane_dimensions.x() = image_plane_width;
	image_plane_dimensions.y() = image_plane_height;
}

/*
 * Compute the backprojection of a point from X,Y and depth plus camera
 */
Eigen::Vector3f backproject(const Camera& camera,
							int pixel_x, 
							int pixel_y, 
						    float depth) 
{
	using namespace Eigen;

	Vector3f cam_origin;
	Vector3f n, u, v;
	construct_camera_coordinate_system(camera, cam_origin, n, u, v);

	Vector3f image_plane_origin;
	Vector2f image_plane_dimensions;
	construct_image_plane_origin(Vector2f{camera.fov[0], camera.fov[1]}, camera.focalDistance, cam_origin, n, u, v, image_plane_origin, image_plane_dimensions);

	// Compute pixel coordinate in world space
	float pixel_width  = image_plane_dimensions.x() / camera.resolution[0];
	float pixel_height = image_plane_dimensions.y() / camera.resolution[1];
	Vector3f pixelCoordinate = image_plane_origin 
								+ ((pixel_x + 0.5f) * pixel_width * u) 
								+ ((pixel_y + 0.5f) * pixel_height * v);

	Vector3f rayDirection = (pixelCoordinate - cam_origin).normalized();;
	return cam_origin + (rayDirection * depth);



//    [h, w] = size(im);
//
//    f_x = (w * 0.5) / tan(deg2rad(fov(2)) * 0.5);
//    f_y = (h * 0.5) / tan(deg2rad(fov(1)) * 0.5);
//
//    u = repmat( 1:w, [h,1]);
//    v = repmat( [1:h]', [1,w]);
//    cx = ones(h,w) * w * 0.5;
//    cy = ones(h,w) * h * 0.5;
//
//    X = ((u - cx) / f_x) .* im;
//    Y = ((v - cy) / f_y) .* im;
//
//    xyz = [X(:), Y(:), im(:)];
//
//    % strip entries which are all zero.
//            xyz = xyz(logical(xyz(:,3)),:);

}

void
backproject(const Camera& camera,int pixel_x, int pixel_y, float depth, float * world_x, float * world_y, float * world_z) {
    Eigen::Vector3f point = backproject(camera, pixel_x, pixel_y, depth);
    *world_x = point(0);
    *world_y = point(1);
    *world_z = point(2);
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


