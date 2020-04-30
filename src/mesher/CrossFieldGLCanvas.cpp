//
// Created by Dave Durbin on 28/3/20.
//

#include "CrossFieldGLCanvas.h"

#include <nanogui/nanogui.h>
#include <Camera/Camera.h>
#include "types.h"
#include "spdlog/spdlog.h"
#include <Geom/geom.h>
#include <algorithm>

const float PI = 3.141592654f;
const float TWO_PI =2 * PI;
const float DEG_TO_RAD = PI / 180.0f;
const float NEAR = 0.1f;
const float FAR = 40.0f;
const int VERTICES_PER_SURFEL = 8;
const float ZOOM_SCALE_FACTOR = 0.05;
const float PAN_SCALE_FACTOR = 0.25;

CrossFieldGLCanvas::~CrossFieldGLCanvas() {
    m_shader.free();
}

CrossFieldGLCanvas::CrossFieldGLCanvas(Widget *parent) : nanogui::GLCanvas(parent),
                                                         m_field_of_view{60.0, 60.0},
                                                         m_focal_length{5},
                                                         m_last_mouse_position{0, 0},
                                                         m_button{-1},
                                                         m_mouse_state{NONE},
                                                         m_theta{0.0f},
                                                         m_phi{PI / 2.0f},
                                                         m_radius{30.0f},
                                                         m_camera_target{0, 0, 0},
                                                         mup{1.0f},
                                                         m_num_surfels{0},
                                                         m_surfel_colouring{NATURAL},
                                                         m_model_matrix(nanogui::Matrix4f::Identity()),
                                                         m_view_matrix(nanogui::Matrix4f::Identity()) {
    using namespace nanogui;

    m_shader.init(
            /* An identifying name */
            "a_simple_shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 modelViewProj;\n"
            "in vec3 position;\n"
            "in vec3 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "    frag_color = /* 3.0 * modelViewProj * */ vec4(color, 1.0);\n"
            "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 color;\n"
            "in vec4 frag_color;\n"
            "void main() {\n"
            "    color = frag_color;\n"
            "}"
    );

    // Initial estimate for up
    make_model_matrix();
    update_view_matrix();
    make_projection();
}

/**
 * Set the colour model to use
 * @param colouring
 */
void
CrossFieldGLCanvas::set_colouring_mode(SurfelColouring colouring) {
    if (m_surfel_colouring != colouring) {
        m_surfel_colouring = colouring;
        set_colours();
    }
}



void CrossFieldGLCanvas::make_model_matrix( ) {
    using namespace nanogui;
    m_model_matrix.setIdentity();
}


void CrossFieldGLCanvas::make_projection( ) {
    const auto fov_rad = m_field_of_view * DEG_TO_RAD;

    const float top = NEAR * tanf(fov_rad.y() * 0.5f);
    const float bottom = -top;
    const float right = NEAR * tanf( fov_rad.x() * 0.5f);
    const float left = -right;

    m_projection_matrix = nanogui::frustum(left, right, bottom, top, NEAR, FAR);
}


nanogui::MatrixXu make_indices(int num_surfels) {
    nanogui::MatrixXu indices(2, num_surfels * 4);
    for (int surfel_idx = 0; surfel_idx < num_surfels; ++surfel_idx) {
        int col_idx = surfel_idx * 4;
        int vertex_idx = surfel_idx * 8;
        indices.col(col_idx + 0) << vertex_idx + 0, vertex_idx + 3;  // Normal
        indices.col(col_idx + 1) << vertex_idx + 1, vertex_idx + 4;  // Principal tangent
        indices.col(col_idx + 2) << vertex_idx + 2, vertex_idx + 5;  // Opposite to principal tangent
        indices.col(col_idx + 3) << vertex_idx + 6, vertex_idx + 7;  // Other tangents
    }
    return indices;
}

nanogui::MatrixXf CrossFieldGLCanvas::make_colours(CrossFieldGLCanvas::SurfelColouring surfel_colouring,
                               int num_surfels,
                               const std::vector<float> &adjustments,
                               const std::vector<float> &errors
        ) {
    assert( !errors.empty());
    assert( !adjustments.empty());

    nanogui::MatrixXf colours(3, 8 * num_surfels);
    float tan_r, tan_g, tan_b = 0.0f;
    float error_scale_factor = 1.0f / (45.0f * 45.0f);
    float error_offset = 0.0f;
    if(surfel_colouring == CrossFieldGLCanvas::ERROR_REL) {
        auto min_error = fabs(*errors.begin());
        auto max_error = min_error;
        for( auto err : errors ) {
            if(fabs(err) < min_error) {
                min_error = fabs(err);
            }
            if(fabs(err) > max_error) {
                max_error = fabs(err);
            }
        }
        auto error_range = max_error - min_error;
        error_scale_factor = (error_range > 0.0 ) ? (1.0f /error_range) : 0.0f;
        error_offset = min_error;
    }



    for (int surfel_idx = 0; surfel_idx < num_surfels; ++surfel_idx) {
        int col_idx = surfel_idx * 8;

        // Handle selected surfel
        const auto it = m_highlight_colours.find(surfel_idx);
        if( it != m_highlight_colours.end()) {
            for( int j=0; j<8; ++j) {
                colours.col(col_idx + j) = it->second;
            }
            continue;
        }

        switch( surfel_colouring) {
            case CrossFieldGLCanvas::ADJUSTMENT:
                // Set tan colour based on adjustment
                // adj is -45 to 45
                tan_r = fabs(adjustments.at(surfel_idx) / 45.0f);
                tan_g = 1.0f - tan_r;
                for( int j=0; j<8; ++j) {
                    colours.col(col_idx + j) << tan_r, tan_g, 0.0;
                }
                break;

            case CrossFieldGLCanvas::NATURAL:
                colours.col(col_idx + 0) << 1, 0, 0; // Red normal
                colours.col(col_idx + 1) << 0, 1, 0; // Green principal tangent
                colours.col(col_idx + 2) << 1, 1, 1; // White other tangents
                colours.col(col_idx + 3) << 1, 0, 0; // Red normal
                colours.col(col_idx + 4) << 0, 1, 0; // Green principal tangent
                colours.col(col_idx + 5) << 1, 1, 1; // White other tangents
                colours.col(col_idx + 6) << 1, 1, 1; // White other tangents
                colours.col(col_idx + 7) << 1, 1, 1; // White other tangents
                break;

            case CrossFieldGLCanvas::ERROR:
            case CrossFieldGLCanvas::ERROR_REL:
                // Set tan colour based on error
                // adj is -45 to 45
                tan_r = (fabs(errors.at(surfel_idx)) - error_offset) * error_scale_factor;
                tan_g = 1.0f - tan_r;
                for( int j=0; j<8; ++j) {
                    colours.col(col_idx + j) << tan_r, tan_g, tan_b;
                }
                break;
        }
    }
    return colours;
}

float
compute_scale_factor(const std::vector<CrossFieldGLCanvas::SurfelData> &surfel_data) {
    std::vector<Eigen::Vector3f> points;
    for( const auto & s : surfel_data) {
        points.push_back(s.point);
    }
    const auto & t = closest_points(points);
    float min_dist = std::get<2>(t);
    return (min_dist > 0.0f) ? min_dist / 2.0f : 1.0f;
}

/**
 * Convert points, normals and tangents into a a series of vertcies to be connected
 * and drawn as lines. Return same as a 3 x (num_surfels *8) matrix
 */
nanogui::MatrixXf
make_vertices(const std::vector<CrossFieldGLCanvas::SurfelData>& surfel_data) {

    auto vector_scale_factor = compute_scale_factor( surfel_data );
    auto num_surfels = surfel_data.size();

    nanogui::MatrixXf vertices(3, VERTICES_PER_SURFEL * num_surfels);
    nanogui::Vector3f centroid{0,0,0};

    unsigned int vertex_idx = 0;
    for (const auto & s : surfel_data) {
        const auto &point = s.point;
        const auto &tan = s.tangent;
        const auto &normal = s.normal;
        centroid = centroid + point;

        vertices.col(vertex_idx + 0) << point.x(), point.y(), point.z(); // Point
        vertices.col(vertex_idx + 1) << point.x(), point.y(), point.z(); // Point
        vertices.col(vertex_idx + 2) << point.x(), point.y(), point.z(); // Point
        const auto p = point + (normal * vector_scale_factor);
        vertices.col(vertex_idx + 3) << p.x(), p.y(), p.z(); // Normal
        const auto t1 = point + (tan * vector_scale_factor);
        vertices.col(vertex_idx + 4) << t1.x(), t1.y(), t1.z(); // Main Tan
        const auto t2 = point - (tan * vector_scale_factor);
        vertices.col(vertex_idx + 5) << t2.x(), t2.y(), t2.z(); // Main Tan

        const auto norm_cross_tan = normal.cross(tan);
        const auto norm_dot_tan = normal.dot(tan);
        const auto kkv = normal * norm_dot_tan;
        const auto tan90 = (norm_cross_tan + kkv);
        const auto &t3 = point - (tan90 * vector_scale_factor);
        const auto &t4 = point + (tan90 * vector_scale_factor);
        vertices.col(vertex_idx + 6) << t3.x(), t3.y(), t3.z(); // Point
        vertices.col(vertex_idx + 7) << t4.x(), t4.y(), t4.z(); // Point

        vertex_idx += VERTICES_PER_SURFEL;
    }
    centroid /= num_surfels;

    // Adjust vertices to be centred at origin
    vertices = vertices.colwise() - centroid;
    return vertices;
}

void CrossFieldGLCanvas::set_colours( ) {
    nanogui::MatrixXf colours = make_colours(m_surfel_colouring, m_num_surfels, m_adjustments, m_errors);

    m_shader.bind();
    m_shader.uploadAttrib("color", colours);
    drawGL();
}

/**
 * Set the current data to render
 * @param points
 * @param normals
 * @param tangents
 * @param adjustments
 * @param errors
 */
void CrossFieldGLCanvas::set_data( const std::vector<SurfelData>& surfel_data) {
    // Store for colours
    m_adjustments.clear();
    m_errors.clear();
    m_points.clear();
    m_num_surfels = surfel_data.size();

    for (const auto &s : surfel_data) {
        m_adjustments.push_back(s.adjustment);
        m_errors.push_back(s.error);
        m_points.push_back(s.point);
    }

    m_shader.bind();
    const auto indices = make_indices(m_num_surfels);
    m_shader.uploadIndices(indices);

    auto positions = make_vertices(surfel_data);
    m_shader.uploadAttrib("position", positions);

    auto colours = make_colours(m_surfel_colouring, m_num_surfels, m_adjustments, m_errors);
    m_shader.uploadAttrib("color", colours);
    drawGL();
}

void CrossFieldGLCanvas::update_view_matrix( ) {
    using namespace nanogui;

    const auto camera_origin = to_cartesian();
    spdlog::debug("Camera now at ({:f}, {:f}, {:f})", camera_origin.x(), camera_origin.y(), camera_origin.z());


    m_view_matrix = lookAt(camera_origin, m_camera_target, Vector3f{0, mup, 0});
    spdlog::debug("\n{:f}, {:f}, {:f}, {:f}\n{:f}, {:f}, {:f}, {:f}\n{:f}, {:f}, {:f}, {:f}\n{:f}, {:f}, {:f}, {:f}\n",
                 m_view_matrix(0, 0), m_view_matrix(0, 1), m_view_matrix(0, 2), m_view_matrix(0, 3),
                 m_view_matrix(1, 0), m_view_matrix(1, 1), m_view_matrix(1, 2), m_view_matrix(1, 3),
                 m_view_matrix(2, 0), m_view_matrix(2, 1), m_view_matrix(2, 2), m_view_matrix(2, 3),
                 m_view_matrix(3, 0), m_view_matrix(3, 1), m_view_matrix(3, 2), m_view_matrix(3, 3));
}

/**
 * Compute a vector for the direction form the camera origin through a given pixel.
 * @param pixel_coord The pixel
 * @return The unit vector
 */
nanogui::Vector3f CrossFieldGLCanvas::compute_ray_through_pixel(const nanogui::Vector2i& pixel_coord ) {
    using namespace nanogui;

    const auto camera_origin = to_cartesian();

    Vector3f N{camera_origin-m_camera_target};
    const auto n = N.normalized();

    // u is a vector that is perpendicular to the plane spanned by
    // N and view up vector (cam->up), ie in the image plane and horizontal
    Vector3f U = Vector3f{0, mup, 0}.cross(n);
    const auto u = U.normalized();

    // v is a vector perpendicular to N and U, i.e vertical in image palne
    const auto v = n.cross(u);

    const auto fov_rad = m_field_of_view * DEG_TO_RAD;
    double image_plane_height = 2 * tan(fov_rad.y() * 0.5f) * m_focal_length;
    double image_plane_width = 2 * tan(fov_rad.x() * 0.5f) * m_focal_length;

    const auto image_plane_centre = camera_origin - (n * m_focal_length);
    const auto image_plane_origin = image_plane_centre - (u * image_plane_width * 0.5f) - (v * image_plane_height * 0.5f);

    // Compute pixel dimensions in world units
    const auto pixel_width = image_plane_width / size().x();
    const auto pixel_height = image_plane_height / size().y();

    // Construct a ray from the camera origin to the world coordinate
    Vector3f pixel_in_world = image_plane_origin
                               + ((pixel_coord.x() + 0.5) * pixel_width * u)
                               + ((pixel_coord.y() + 0.5) * pixel_height * v);

    return  (pixel_in_world - camera_origin).normalized();
}


void
CrossFieldGLCanvas::select_surfel(unsigned int surfel_idx) {
    if(m_click_callback) {
        m_click_callback(surfel_idx);
    } else {
        spdlog::debug("No callback defined");
    }
}

void
CrossFieldGLCanvas::handle_mouse_click(const nanogui::Vector2i& window_coords) {
    using namespace nanogui;

    // Convert coords to canvas
    const auto screen_coords = window_coords - position();
    spdlog::info("Click at {:d}, {:d}", screen_coords.x(), screen_coords.y());

    // Now create a ray from Cam origin through this pixel
    const auto ray = compute_ray_through_pixel(Vector2i{screen_coords.x(), size().y() - screen_coords.y()-1});
    spdlog::info("   Ray {:f}, {:f}, {:f}", ray.x(), ray.y(), ray.z());

    const auto camera_origin = to_cartesian();

    float min_dist = std::numeric_limits<float>::max();
    unsigned int closest_surfel_idx = 0;
    unsigned int surfel_idx = 0;
    for( const auto& pt : m_points ) {
        auto dist = distance_from_point_to_line(pt, camera_origin, ray);
        if(dist < min_dist) {
            min_dist = dist;
            closest_surfel_idx = surfel_idx;
        }
        ++surfel_idx;
    }
    select_surfel(closest_surfel_idx);
}

/*
 * State
 * Mouse up -> Mouse down
 * Mouse down -> drag or up or zoom
 * Drag -> up
 * Zoom -> up
 */
bool
CrossFieldGLCanvas::mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) {
    // If button up without drag or zoom it's a click
    m_last_mouse_position = p;
    if( down ) {
        m_button = button;
        m_mouse_state = DOWN;
    } else {
        if( m_mouse_state == DOWN ) {
            handle_mouse_click(m_last_mouse_position);
        }
        m_mouse_state = NONE;
    }
    return true;
}

bool
CrossFieldGLCanvas::mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button,
                                   int modifiers) {
    if ( m_mouse_state == DOWN ) {
        if (m_button == 0) {
            if ((modifiers & GLFW_MOD_SUPER) != 0) {
                m_mouse_state = ZOOMING;
            } else if ((modifiers & GLFW_MOD_ALT) != 0) {
                m_mouse_state = PANNING;
            } else {
                m_mouse_state = ROTATING;
            }
        }
    }

    switch (m_mouse_state) {
        case PANNING:
            pan(-rel.x() * PAN_SCALE_FACTOR, rel.y() * PAN_SCALE_FACTOR);
            break;
        case ZOOMING: {
            auto zm = fabs(rel.x()) > fabs(rel.y())
                      ? rel.x()
                      : rel.y();
            zoom(zm * ZOOM_SCALE_FACTOR);
        }
            break;
        case ROTATING:
            rotate(-rel.x() / 100.0f, -rel.y() / 100.0f);
            break;
        default:
            break;
    }
    return true;
}



void CrossFieldGLCanvas::drawGL() {
    using namespace nanogui;
    m_shader.bind();

    Matrix4f transform = m_projection_matrix * m_view_matrix * m_model_matrix;
    m_shader.setUniform("modelViewProj", transform);

    glEnable(GL_DEPTH_TEST);
    m_shader.drawIndexed(GL_LINES, 0, m_num_surfels * 4);
    glDisable(GL_DEPTH_TEST);
}

void CrossFieldGLCanvas::pan(float d_x, float d_y){
    using namespace nanogui;
    const auto look = (m_camera_target - to_cartesian()).normalized();
    const auto right = look.cross(Vector3f{0, mup, 0});
    const auto up = look.cross(right);
    m_camera_target = m_camera_target + (right * d_x) + (up * d_y);
    update_view_matrix();
}

// Cam handling
void CrossFieldGLCanvas::rotate(float d_theta, float d_phi){
    if (mup > 0.0f) {
        m_theta += d_theta;
    } else {
        m_theta -= d_theta;
    }

    m_phi += d_phi;

    // Keep phi within -2PI to +2PI for easy 'up' comparison
    if (m_phi > TWO_PI) {
        m_phi -= TWO_PI;
    } else if (m_phi < -TWO_PI) {
        m_phi += TWO_PI;
    }

    // If phi is between 0 to PI or -PI to -2PI, make 'up' be positive Y, other wise make it negative Y
    if ((m_phi > 0 && m_phi < PI) || (m_phi < -PI && m_phi > -TWO_PI)) {
        mup = 1.0f;
    } else {
        mup = -1.0f;
    }
    update_view_matrix();

}

void CrossFieldGLCanvas::centre(){
    m_radius = 30;
    m_camera_target << 0.0, 0.0, 0.0;
    m_theta = 0.0f;
    m_phi = PI / 2.0f;
    update_view_matrix();
}


void CrossFieldGLCanvas::zoom(float distance){
    m_radius -= distance;
    // Don't let the radius go negative
    // If it does, re-project our target down the look vector
    if (m_radius <= 0.0f) {
        m_radius = 30.0f;
        auto look = (m_camera_target - to_cartesian()).normalized();
        m_camera_target = m_camera_target + (30.0f * look);
    }
    update_view_matrix();
}