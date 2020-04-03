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

    void to_pixel_and_depth(const Eigen::Vector3f& world_coordinate, unsigned int& pixel_x, unsigned int& pixel_y, float& depth) const;

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

    inline Eigen::Vector3f
    origin() const {return m_origin;}


    inline Eigen::Vector3f
    look_at() const { return m_look_at;}

    inline Eigen::Vector2f
    field_of_view() const { return m_field_of_view;}

    inline Eigen::Vector2f
    resolution() const { return m_resolution;}

    inline Eigen::Vector3f
    up() const { return m_up;}

    inline float
    focal_length() const { return m_focal_length;}

    Eigen::Matrix3f intrinsic_matrix( ) const;

private:
    Eigen::Vector3f m_origin;               // World Coordinates
    Eigen::Vector3f m_look_at;              // World Coordinates
    Eigen::Vector2f m_field_of_view;        // Radians
    Eigen::Vector2f m_resolution;           // Pixel size of image
    Eigen::Vector3f m_up;
    Eigen::Vector3f image_plane_origin;     // World Coordinates
    Eigen::Vector2f image_plane_dimensions; // World units
    float m_focal_length{};                   // World units
    double pixel_width{};                     // World units
    double pixel_height{};                    // World units
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
    void compute_camera_parms();

    friend std::ostream &operator<<(std::ostream &os, const Camera &camera);

    friend Camera scale_camera( const Camera& source_camera, float scale_x, float scale_y ) {
        Camera lc{source_camera.m_origin.data(),
                  source_camera.m_look_at.data(),
                  source_camera.m_up.data(),
                  source_camera.m_resolution.data(),
                  source_camera.m_field_of_view.data(),
                  source_camera.m_focal_length};
        lc.m_resolution[0] /= scale_x;
        lc.m_resolution[1] /= scale_y;
        return lc;
    }
};


Camera loadCameraFromFile(const std::string &filename);

std::ostream &operator<<(std::ostream &os, const Camera &camera);