#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <Eigen/Geometry>
#include <Camera/Camera.h>

const double DEG_TO_RAD = M_PI / 180.0;

Camera::Camera(const float position[3],
               const float view[3], const float up[3], const float res[2],
               const float fov[2], float f) : camera_origin{position[0], position[1], position[2]},
                                              looking_at{view[0], view[1], view[2]},
                                              up{up[0], up[1], up[2]},
                                              resolution{res[0], res[1]},
                                              field_of_view{fov[0] * DEG_TO_RAD, fov[1] * DEG_TO_RAD},
                                              focal_length{f} {
    compute_camera_parms();
}

void
Camera::set_image_size( unsigned int width, unsigned int height ) {
    // Compute pixel dimensions in world units
    resolution.x() = width;
    resolution.y() = height;
    pixel_width = image_plane_dimensions.x() / resolution.x();
    pixel_height = image_plane_dimensions.y() / resolution.y();
}

void
Camera::compute_camera_parms() {
    using namespace Eigen;

    // n is normal to image plane, points in opposite direction of view point
    Vector3f N{camera_origin.x() - looking_at.x(),
               camera_origin.y() - looking_at.y(),
               camera_origin.z() - looking_at.z()};
    n = N.normalized();

    // u is a vector that is perpendicular to the plane spanned by
    // N and view up vector (cam->up), ie in the image plane and horizontal
    Vector3f U = up.cross(n);
    u = U.normalized();

    // v is a vector perpendicular to N and U, i.e vertical in image palne
    v = n.cross(u);

    // Compute dimensions of image plane in world units
    /*
     * We have
     *
     * +          ^
     * |\         |
     * |a\        |
     * |  \       f = F*pixels per world unit
     * |   \      |
     * +----+     v
     * pix width /w
     * tan(a) = opp/adj = (image_width_pixels * 0.5) / focal_length_pixels
     * 2tan(a) = image_width_pixels / focal_length_pixels
     * => focal_length_pixels = image_width_pixels / (2tan(a))
     *
     * pixels_per_world_unit = focal_length_pixels / focal_length_world_units
     *  focal_length_pixels = pixels_per_world_unit / focal_length_world_units
     *
     *  image_width_pixels / (2tan(a)) = pixels_per_world_unit / focal_length_world_units
     *  => pixels_per_world_unit = (image_width_pixels / (2tan(a))) *  focal_length_world_units
     */
    double pixels_per_wux = (resolution.x() / (2 * tan(field_of_view.x() * 0.5f))) * focal_length;
    double pixels_per_wuy = (resolution.y() / (2 * tan(field_of_view.y() * 0.5f))) * focal_length;

    double image_plane_height = pixels_per_wuy / resolution.y();
    double image_plane_width = pixels_per_wux / resolution.x();

    Vector3f image_plane_centre = camera_origin - (n * focal_length);
    image_plane_origin = image_plane_centre - (u * image_plane_width * 0.5f) - (v * image_plane_height * 0.5f);

    // Compute pixel dimensions in world units
    pixel_width = image_plane_width / resolution.x();
    pixel_height = image_plane_height / resolution.y();

    image_plane_dimensions.x() = image_plane_width;
    image_plane_dimensions.y() = image_plane_height;
}


void parseVec3(const std::string &str, float vec[3]) {
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

void parseVec2(const std::string &str, float vec[2]) {
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

Camera loadCameraFromFile(const std::string &filename) {
    using namespace std;
    using namespace Eigen;

    bool fl_position, fl_view, fl_up, fl_resolution, fl_fov, fl_f;
    fl_position = fl_view = fl_up = fl_resolution = fl_fov = fl_f = false;

    ifstream file;
    file.exceptions(ifstream::failbit | ifstream::badbit);

    try {
        file.open(filename);
    }
    catch (ifstream::failure& e) {
        cout << "ERROR::CAMFILE::NOT_FOUND " << filename << endl;
        cerr << strerror(errno) << endl;
        throw runtime_error("No camera file found" );
    }

    float position[3];
    float view_direction[3];
    float up_direction[3];
    float resolution[2];
    float field_of_view[2];
    float focal_length;

    file.exceptions(ifstream::goodbit);
    string line;
    while (getline(file, line)) {
        istringstream is_line(line);
        string key;
        if (getline(is_line, key, '=')) {
            string value;
            if (getline(is_line, value)) {
                // Handle the line
                if (key == "position") {
                    parseVec3(value, position);
                    fl_position = true;
                } else if (key == "view") {
                    parseVec3(value, view_direction);
                    fl_view = true;
                } else if (key == "up") {
                    parseVec3(value, up_direction);
                    fl_up = true;
                } else if (key == "resolution") {
                    parseVec2(value, resolution);
                    fl_resolution = true;
                } else if (key == "fov") {
                    parseVec2(value, field_of_view);
                    fl_fov = true;
                } else if (key == "f") {
                    focal_length = stof(value);
                    fl_f = true;
                } else {
                    cerr << "ERROR::CAMFILE::UNKNOWN_KEY " << key << endl;
                    throw std::domain_error("CAMFILE::UNKNOWN_KEY");
                }
            }
        }
    }

    if (!(fl_position && fl_view && fl_up && fl_resolution && fl_fov && fl_f)) {
        cerr << "ERROR::CAMFILE::MISSING_KEY" << endl;
        throw std::domain_error("CAMFILE::MISSING_KEY");
    }

    return Camera{position, view_direction, up_direction, resolution, field_of_view, focal_length};
}

/*
 * Get the camera matrix.
 */
void Camera::camera_intrinsics(Eigen::Matrix3f &K) {
    double cx = resolution.x() * 0.5;
    double cy = resolution.y() * 0.5;
    double fx = resolution.x() / tan(field_of_view.x() * 0.5);
    double fy = resolution.y() / tan(field_of_view.y() * 0.5);
    float skew = 0.0f;
    K << fx, skew, cx,
            0.0f, fy, cy,
            0.0f, 0.0f, 1.0f;
}

/**
 * Based on https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
 */
void
Camera::camera_extrinsics(Eigen::Matrix3f &R, Eigen::Vector3f &t) {
    using namespace Eigen;

    R << v.x(), u.x(), n.x(),
            v.y(), u.y(), n.y(),
            v.z(), u.z(), n.z();

    t = -R * camera_origin;
}

/**
 * Break camera down i to extrinsic and intrinsic matrices
 */
void
Camera::decompose(Eigen::Matrix3f &K, Eigen::Matrix3f &R, Eigen::Vector3f &t) {
    camera_intrinsics(K);
    camera_extrinsics(R, t);
    std::cout << "Cam R " << R << std::endl;
    std::cout << "Cam t " << t << std::endl;
}

/**
 * Friend function for dumping Camera objects
 */
std::ostream
&operator<<(std::ostream &os, const Camera &camera) {
    using namespace std;
    os << "pos : " << camera.camera_origin.x() << ", " << camera.camera_origin.y() << ", " << camera.camera_origin.z() << endl;
    os << "vew : " << camera.n.x() << ", " << camera.n.y() << ", " << camera.n.z() << endl;
    os << " up : " << camera.v.x() << ", " << camera.v.y() << ", " << camera.v.z() << endl;
    os << "res : " << camera.resolution.x() << ", " << camera.resolution.y() << endl;
    os << "fov : " << camera.field_of_view.x() << ", " << camera.field_of_view.y() << endl;
    os << "foc : " << camera.focal_length << endl;
    return os;
}

/*
 * Compute the backprojection of a point from X,Y and depth in world space
 */
Eigen::Vector3f
Camera::to_world_coordinates(unsigned int pixel_x, unsigned int pixel_y, float depth) const {
    using namespace Eigen;

    // Get world coordinates of pixel through which the ray passes
   Vector3f pixelCoordinate = image_plane_origin
                               + ((pixel_x /*+ 0.5*/) * pixel_width * u)
                               + ((pixel_y /*+ 0.5*/) * pixel_height * v);

   // Now project the ray to the depth expected to give the final world coordinate of the projected point
    Vector3f rayDirection = (pixelCoordinate - camera_origin).normalized();
    return camera_origin + (rayDirection * depth);
}

/*
 * Convenience method to to_world_coords a point into an array
 */
void
Camera::to_world_coordinates(unsigned int pixel_x, unsigned int pixel_y, float depth, float *world_coordinate) const {
    assert( world_coordinate != nullptr);
    Eigen::Vector3f point = to_world_coordinates(pixel_x, pixel_y, depth);
    world_coordinate[0] = point(0);
    world_coordinate[1] = point(1);
    world_coordinate[2] = point(2);
}

/**
 * Change the location of the camera in world coordinates.
 * If keep_facing is true, the camera stays pointed in the same direction as it currently is
 * otherwise, it rotates to stay pointing at the same point in space.
 */
void
Camera::move_to( float world_x, float world_y, float world_z, bool keep_facing ) {
    camera_origin << world_x, world_y, world_z;
    if( keep_facing) {
        looking_at << world_x - n.x(), world_y - n.y(), world_z - n.z();
    }
    compute_camera_parms();
}

/**
 * Change the point that the camera is looking at
 */
void
Camera::look_at( float view_x, float view_y, float view_z ) {
    looking_at << view_x, view_y, view_z;
    compute_camera_parms();
}

