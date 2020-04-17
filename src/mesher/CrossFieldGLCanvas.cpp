//
// Created by Dave Durbin on 28/3/20.
//

#include "CrossFieldGLCanvas.h"

#include <nanogui/nanogui.h>
#include <Camera/Camera.h>
#include "types.h"
#include "spdlog/spdlog.h"


const float VECTOR_SCALE_FACTOR = .1f;

CrossFieldGLCanvas::~CrossFieldGLCanvas() {
    m_shader.free();
}

CrossFieldGLCanvas::CrossFieldGLCanvas(Widget *parent) : nanogui::GLCanvas(parent) {
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

    m_arcball.setSize(size());// Note 1

    make_projection( );
    update_mvp();
}


void CrossFieldGLCanvas::make_projection( ) {
    const float pi = 4.0f * atanf(1.0);
    const float near = 0.1f;
    const float far = 15.0f;
    const float fov = 60; // degrees
    const float aspect = width() / height();

    const float top = near * tan( (fov / 2.0) * (pi / 180.0f));
    const float bottom = -top;
    const float right = top * aspect;
    const float left = -right;

    // Convert fovy to radians, then divide by 2
    m_projection << (2.0f * near) / (right -left), 0, (right+left)/(right-left), 0,
    0, (2 * near) / (top - bottom), (top + bottom) / (top-bottom), 0,
    0, 0, -(far + near) / (far - near), -(2.0f * far * near) / (far - near),
    0, 0, -1, 0;
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

nanogui::MatrixXf make_colours(CrossFieldGLCanvas::SurfelColouring surfel_colouring,
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
                    colours.col(col_idx + j) << tan_r, tan_g, 0.0;
                }
                break;
        }
    }
    return colours;
}

nanogui::MatrixXf CrossFieldGLCanvas::make_vertices(
        const std::vector<std::vector<nanogui::Vector3f>> &points,
        const std::vector<std::vector<nanogui::Vector3f>> &normals,
        const std::vector<std::vector<nanogui::Vector3f>> &tangents) {

    auto num_surfels = points.at(m_frame_idx).size();
    nanogui::MatrixXf vertices(3, 8 * num_surfels);

    m_centroid.setZero();
    for (unsigned int surfel_idx = 0; surfel_idx < num_surfels; ++surfel_idx) {
        unsigned int vertex_idx = surfel_idx * 8;
        const auto &point = points.at(m_frame_idx).at(surfel_idx);
        const auto &tan = tangents.at(m_frame_idx).at(surfel_idx);
        const auto &normal = normals.at(m_frame_idx).at(surfel_idx);
        m_centroid = m_centroid + point;

        vertices.col(vertex_idx + 0) << point.x(), point.y(), point.z(); // Point
        vertices.col(vertex_idx + 1) << point.x(), point.y(), point.z(); // Point
        vertices.col(vertex_idx + 2) << point.x(), point.y(), point.z(); // Point
        const auto p = point + (normal * VECTOR_SCALE_FACTOR);
        vertices.col(vertex_idx + 3) << p.x(), p.y(), p.z(); // Normal
        const auto t1 = point + (tan * VECTOR_SCALE_FACTOR);
        vertices.col(vertex_idx + 4) << t1.x(), t1.y(), t1.z(); // Main Tan
        const auto t2 = point - (tan * VECTOR_SCALE_FACTOR);
        vertices.col(vertex_idx + 5) << t2.x(), t2.y(), t2.z(); // Main Tan

        const auto norm_cross_tan = normal.cross(tan);
        const auto norm_dot_tan = normal.dot(tan);
        const auto kkv = normal * norm_dot_tan;
        const auto tan90 = (norm_cross_tan + kkv);
        const auto &t3 = point - (tan90 * VECTOR_SCALE_FACTOR);
        const auto &t4 = point + (tan90 * VECTOR_SCALE_FACTOR);
        vertices.col(vertex_idx + 6) << t3.x(), t3.y(), t3.z(); // Point
        vertices.col(vertex_idx + 7) << t4.x(), t4.y(), t4.z(); // Point
    }
    m_centroid /= num_surfels;

    // Adjust vertices to be centred ato origin
    vertices = vertices.colwise() - m_centroid;
    return vertices;
}

void CrossFieldGLCanvas::set_colours( ) {
    nanogui::MatrixXf colours = make_colours(m_surfel_colouring, m_num_surfels, m_adjustments, m_errors);
    m_shader.bind();
    m_shader.uploadAttrib("color", colours);
}

void CrossFieldGLCanvas::load_gl_data( ) {
    using namespace nanogui;
    using namespace std;

    m_num_surfels = m_points.at(m_frame_idx).size();

    MatrixXu indices = make_indices(m_num_surfels);
    MatrixXf positions = make_vertices(m_points, m_normals, m_tangents);
    MatrixXf colours = make_colours(m_surfel_colouring, m_num_surfels, m_adjustments, m_errors);

    m_shader.bind();
    m_shader.uploadIndices(indices);

    m_shader.uploadAttrib("position", positions);
    m_shader.uploadAttrib("color", colours);
}

void CrossFieldGLCanvas::set_data(const std::vector<std::vector<nanogui::Vector3f>> &points,
                                  const std::vector<std::vector<nanogui::Vector3f>> &normals,
                                  const std::vector<std::vector<nanogui::Vector3f>> &tangents,
                                  const std::vector<float> &adjustments,
                                  const std::vector<float> &errors) {
    m_adjustments = adjustments;
    m_errors = errors;
    m_points = points;
    m_tangents = tangents;
    m_normals = normals;

    load_gl_data();
}

void CrossFieldGLCanvas::update_mvp( ) {
    using namespace nanogui;
    Vector3f origin = (m_zoom / sqrt(3.0)) * Vector3f{1, 1, 1};
    Vector3f up{0,1,0};
    const auto forward = -origin;
    const auto right = forward.cross(up);
    up = forward.cross(right);

    m_mvp = m_projection * nanogui::lookAt(
            origin,
            Vector3f{0, 0, 0},
            up
    );
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
    if( down ) {
        m_mouse_down = p;
        m_zooming = false;
        m_dragging = false;
        m_button = button;
    } else {
        if( m_dragging )  {
            m_dragging = false;
            m_arcball.button(p, false);
        } else if ( m_zooming) {
            m_zooming = false;
        }
        else {
            // Click!
            spdlog::info("Click at {:d}, {:d}", m_mouse_down.x(), m_mouse_down.y());
        }
    }
    return true;
}

bool
CrossFieldGLCanvas::mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button,
                                   int modifiers) {
    if( !m_zooming && !m_dragging) {
         // Start zoom/drag
        if (m_button == 0) {
            m_arcball.button(m_mouse_down, true);
            m_dragging = true;
        } else if (m_button == 1) {
            m_zooming = true;
        }
    }

    if (m_zooming) {
        float delta = (float) rel.y() / m_arcball.size().y();
        setZoom(m_zoom
                + ((MAX_ZOOM - MIN_ZOOM) * delta));
    } else {
        m_arcball.
                motion(p);// Note 2
    }
    return true;
}

void CrossFieldGLCanvas::drawGL() {
    using namespace nanogui;
    m_shader.bind();

    Matrix4f rx;
    rx << 1, 0, 0, 0,
            0, cos(m_rotx), -sin(m_rotx), 0,
            0, sin(m_rotx), cos(m_rotx), 0,
            0, 0, 0, 1;
    Matrix4f ry;
    ry << cos(m_roty), 0, sin(m_roty), 0,
            0, 1, 0, 0,
            -sin(m_roty), 0, cos(m_roty), 0,
            0, 0, 0, 1;
    Matrix4f rz;
    rz << cos(m_rotz), -sin(m_rotz), 0, 0,
            sin(m_rotz), cos(m_rotz), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;

    auto rot = m_arcball.matrix();
    Matrix4f transform = m_mvp * rot;
    m_shader.setUniform("modelViewProj", transform);

    glEnable(GL_DEPTH_TEST);
    m_shader.drawIndexed(GL_LINES, 0, m_num_surfels * 4);
    glDisable(GL_DEPTH_TEST);
}

