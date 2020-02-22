#include <ostream>
#include <string>
#include <Eigen/Core>

#pragma once

class Camera {
public:
    Camera() = default;

    Camera(const float pos[3], const float v[3], const float u[3], const float res[2], const float fv[2], float f);

    void move_to( float world_x, float world_y, float world_z, bool keep_facing );

    void look_at( float world_x, float world_y, float world_z );

    void set_image_size( unsigned int width, unsigned int height );

    void to_world_coordinates(unsigned int pixel_x, unsigned int pixel_y, float depth, float *world_coordinate) const;

    Eigen::Vector3f to_world_coordinates(unsigned int pixel_x, unsigned int pixel_y, float depth) const;

    /**
     * Get the camera matrix
     */
    void camera_intrinsics(Eigen::Matrix3f &K);
    /**
     * Based on https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
     */
    void
    camera_extrinsics(Eigen::Matrix3f &R, Eigen::Vector3f &t);

    void
    decompose(Eigen::Matrix3f &K, Eigen::Matrix3f &R, Eigen::Vector3f &t);

    inline const Eigen::Vector3f
    origin() const {return camera_origin;}

private:
    Eigen::Vector3f camera_origin;          // World Coordinates
    Eigen::Vector3f looking_at;             // World Coordinates
    Eigen::Vector2f field_of_view;          // Radians
    Eigen::Vector3f image_plane_origin;     // World Coordinates
    Eigen::Vector2f image_plane_dimensions; // World units
    Eigen::Vector2f resolution;             // Pixel size of image
    float focal_length;                     // World units
    double pixel_width;                      // World units
    double pixel_height;                     // World units
    Eigen::Vector3f up;
    Eigen::Vector3f n;
    Eigen::Vector3f u;
    Eigen::Vector3f v;

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
    void construct_camera_coordinate_system();
    void construct_image_plane_origin();
    void compute_camera_parms();

    friend std::ostream &operator<<(std::ostream &os, const Camera &camera);

    friend Camera scale_camera( const Camera& source_camera, float scale_x, float scale_y ) {
        const float *position = source_camera.camera_origin.data();
        const float *up = source_camera.up.data();
        const float *resolution = source_camera.resolution.data();
        const float *fov = source_camera.field_of_view.data();
        Camera lc{position,
                  source_camera.looking_at.data(),
                  up,
                  resolution,
                  fov,
                  source_camera.focal_length};
        lc.resolution[0] /= scale_x;
        lc.resolution[1] /= scale_y;
        return lc;
    }
};


Camera loadCameraFromFile(const std::string &filename);

std::ostream &operator<<(std::ostream &os, const Camera &camera);