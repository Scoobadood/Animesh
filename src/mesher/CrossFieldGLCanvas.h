//
// Created by Dave Durbin on 28/3/20.
//

#ifndef ANIMESH_CROSSFIELDGLCANVAS_H
#define ANIMESH_CROSSFIELDGLCANVAS_H

#include <nanogui/nanogui.h>
#include <Camera/Camera.h>
#include <Eigen/src/Core/Matrix.h>
#include "types.h"

const float MIN_ZOOM = 0.1f;
const float MAX_ZOOM = 20.0f;

class CrossFieldGLCanvas : public nanogui::GLCanvas {
public:
    CrossFieldGLCanvas(Widget *parent);

    ~CrossFieldGLCanvas();

    virtual void drawGL() override;

    void update_mvp();

    void setFrame(int frame) {
        m_frame_idx = frame;
        load_gl_data();
    }

    void setZoom(float zoom) {
        m_zoom = zoom;
        update_mvp();
    }

    void set_data(const std::vector<std::vector<nanogui::Vector3f>> &points,
                  const std::vector<std::vector<nanogui::Vector3f>> &normals,
                  const std::vector<std::vector<nanogui::Vector3f>> &tangents,
                  const std::vector<float> &adjustments,
                  const std::vector<float> &errors);
    enum SurfelColouring {
        NATURAL,
        ADJUSTMENT,
        ERROR,
        ERROR_REL
    };

    void set_colouring_mode(SurfelColouring colouring) {
        if (m_surfel_colouring != colouring) {
            m_surfel_colouring = colouring;
            set_colours();
        }
    }

private:
    int m_num_surfels;
    nanogui::GLShader m_shader;
    float m_rotx;
    float m_roty;
    float m_rotz;
    float m_zoom = MIN_ZOOM;
    bool m_zooming = false;
    bool m_dragging = false;
    nanogui::Vector2i m_mouse_down;
    int m_button;

    SurfelColouring m_surfel_colouring;

    std::vector<std::vector<nanogui::Vector3f>> m_points;
    std::vector<std::vector<nanogui::Vector3f>> m_normals;
    std::vector<std::vector<nanogui::Vector3f>> m_tangents;
    std::vector<float> m_adjustments;
    std::vector<float> m_errors;


    nanogui::Vector3f m_centroid;

    size_t m_frame_idx = 0;
    nanogui::Matrix4f m_projection;
    nanogui::Matrix4f m_mvp;

    nanogui::MatrixXf make_vertices(
            const std::vector<std::vector<nanogui::Vector3f>> &points,
            const std::vector<std::vector<nanogui::Vector3f>> &normals,
            const std::vector<std::vector<nanogui::Vector3f>> &tangents);

    void load_gl_data();

    void make_projection();

    void set_colours();

    nanogui::Arcball m_arcball;

    bool mouseButtonEvent(const nanogui::Vector2i &p, int button, bool down, int modifiers) override;

    bool mouseDragEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;
};


#endif //ANIMESH_CROSSFIELDGLCANVAS_H
