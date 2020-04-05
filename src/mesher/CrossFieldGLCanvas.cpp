//
// Created by Dave Durbin on 28/3/20.
//

#include "CrossFieldGLCanvas.h"

#include <nanogui/nanogui.h>
#include <Camera/Camera.h>
#include "types.h"

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

nanogui::MatrixXf make_colours(int num_surfels) {
    nanogui::MatrixXf colours(3, 8 * num_surfels);
    for (int surfel_idx = 0; surfel_idx < num_surfels; ++surfel_idx) {
        int col_idx = surfel_idx * 8;
        colours.col(col_idx + 0) << 1, 0, 0; // Red normal
        colours.col(col_idx + 1) << 1, 1, 1; // White principal tangent
        colours.col(col_idx + 2) << 0, 1, 0; // Green other tangents
        colours.col(col_idx + 3) << 1, 0, 0; // Red normal
        colours.col(col_idx + 4) << 0, 1, 1; // White principal tangent
        colours.col(col_idx + 5) << 0, 1, 0; // Green other tangents
        colours.col(col_idx + 6) << 0, 1, 0; // Green other tangents
        colours.col(col_idx + 7) << 0, 1, 0; // Green other tangents
    }
    return colours;
}

nanogui::MatrixXf CrossFieldGLCanvas::make_vertices(
        const std::vector<std::vector<nanogui::Vector3f>> &points,
        const std::vector<std::vector<nanogui::Vector3f>> &normals,
        const std::vector<std::vector<nanogui::Vector3f>> &tangents) {

    auto num_surfels = points.at(m_frame_idx).size();
    nanogui::MatrixXf vertices(3, 8 * num_surfels);

    m_centroid.Zero();
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
    std::cout << "centroid " << m_centroid << std::endl;

    // Adjust vertices to be centred ato origin
    vertices = vertices.colwise() - m_centroid;
    return vertices;
}

void CrossFieldGLCanvas::load_gl_data( ) {
    using namespace nanogui;
    using namespace std;

    m_num_surfels = m_points.at(m_frame_idx).size();

    MatrixXu indices = make_indices(m_num_surfels);
    MatrixXf positions = make_vertices(m_points, m_normals, m_tangents);
    MatrixXf colours = make_colours(m_num_surfels);

    m_shader.bind();
    m_shader.uploadIndices(indices);

    m_shader.uploadAttrib("position", positions);
    m_shader.uploadAttrib("color", colours);
}

void CrossFieldGLCanvas::set_data(const std::vector<std::vector<nanogui::Vector3f>> &points,
                                  const std::vector<std::vector<nanogui::Vector3f>> &normals,
                                  const std::vector<std::vector<nanogui::Vector3f>> &tangents) {
    m_points = points;
    m_tangents = tangents;
    m_normals = normals;

    load_gl_data();
}

void CrossFieldGLCanvas::update_mvp( ) {
    using namespace nanogui;
    Vector3f origin = (m_radius / sqrt(3.0)) * Vector3f{1, 1, 1};
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

    auto rot = rx * ry * rz;
    Matrix4f transform = m_mvp * rot;
    m_shader.setUniform("modelViewProj", transform);

    glEnable(GL_DEPTH_TEST);
    m_shader.drawIndexed(GL_LINES, 0, m_num_surfels * 4);
    glDisable(GL_DEPTH_TEST);
}

