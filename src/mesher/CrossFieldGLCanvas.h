//
// Created by Dave Durbin on 28/3/20.
//

#ifndef ANIMESH_CROSSFIELDGLCANVAS_H
#define ANIMESH_CROSSFIELDGLCANVAS_H

#include <nanogui/nanogui.h>
#include <Camera/Camera.h>
#include <Eigen/src/Core/Matrix.h>

#include <utility>
#include "types.h"

class CrossFieldGLCanvas : public nanogui::GLCanvas {
public:
    struct SurfelData {
        nanogui::Vector3f point;
        nanogui::Vector3f normal;
        nanogui::Vector3f tangent;
        float adjustment;
        float error;

        SurfelData(
                const nanogui::Vector3f& point,
                const nanogui::Vector3f& normal,
                const nanogui::Vector3f& tangent,
                float adjustment,
                float error) : point{point}, normal{std::move(normal)}, tangent{std::move(tangent)}, adjustment{adjustment}, error{error} {}
    };

    /**
     * How to colour surfels
     */
    enum SurfelColouring {
        NATURAL,
        ADJUSTMENT,
        ERROR,
        ERROR_REL
    };

    explicit CrossFieldGLCanvas(Widget *parent);

    ~CrossFieldGLCanvas() override;

    void drawGL() override;

    /**
     * Set the data to render.
     * @param points The surfel coords
     * @param normals Directions of the normals
     * @param tangents Directions of the principal tangents
     * @param adjustments Amount by which the data was most recently adjusted
     * @param errors Error residual per surfel.
     */

    void set_data(const std::vector<SurfelData> &surfel_data);

    /**
     * Set the colour model to use
     * @param colouring
     */
    void set_colouring_mode(SurfelColouring colouring);

    nanogui::MatrixXf make_colours(CrossFieldGLCanvas::SurfelColouring surfel_colouring,
                                   int num_surfels,
                                   const std::vector<float> &adjustments,
                                   const std::vector<float> &errors);

    void set_click_callback(std::function<void(int)> fn) {
        m_click_callback = std::move(fn);
    }

    void highlight_surfel(int surfel_idx, nanogui::Vector3f colour) {
        assert(surfel_idx >= 0 && surfel_idx < m_num_surfels);
        auto it = m_highlight_colours.find(surfel_idx);
        if (it == m_highlight_colours.end()) {
            m_highlight_colours.emplace(surfel_idx, colour);
        } else {
            it->second = colour;
        }
        set_colours();
    }

    void remove_highlights() {
        m_highlight_colours.clear();
        set_colours();
    }

    void centre();

private:
    void update_view_matrix();

    // Projection Parameters
    const nanogui::Vector2f m_field_of_view;
    const float m_focal_length;

    // Mouse handling
    nanogui::Vector2i m_last_mouse_position;
    int m_button;
    enum {
        NONE,
        DOWN,
        PANNING,
        ZOOMING,
        ROTATING
    } m_mouse_state;

    // Camera position and orientation
    float m_theta;
    float m_phi;
    float m_radius;
    nanogui::Vector3f m_camera_target;
    float mup;

    std::function<void(int)> m_click_callback;

    // Surfel rendering
    int m_num_surfels;
    nanogui::GLShader m_shader;
    SurfelColouring m_surfel_colouring;
    std::vector<nanogui::Vector3f> m_points;
    std::vector<float> m_adjustments;
    std::vector<float> m_errors;
    std::map<unsigned int, nanogui::Vector3f> m_highlight_colours;

    // GL Matrices
    nanogui::Matrix4f m_projection_matrix;
    nanogui::Matrix4f m_model_matrix;
    nanogui::Matrix4f m_view_matrix;

    void make_model_matrix();

    // Cam handling
    void rotate(float d_theta, float d_phi);

    void zoom(float distance);

    inline nanogui::Vector3f to_cartesian() {
        float x = m_radius * sinf(m_phi) * sinf(m_theta);
        float y = m_radius * cosf(m_phi);
        float z = m_radius * sinf(m_phi) * cosf(m_theta);
        return m_camera_target + nanogui::Vector3f{x, y, z};
    }

    void pan(float d_x, float d_y);

    void make_projection();

    void set_colours();

    void select_surfel(unsigned int surfel_idx);

    bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;

    bool mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;

    void handle_mouse_click(const nanogui::Vector2i &window_coords);

    nanogui::Vector3f compute_ray_through_pixel(const nanogui::Vector2i &pixel_coord);
};


#endif //ANIMESH_CROSSFIELDGLCANVAS_H
