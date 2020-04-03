//
// Created by Dave Durbin on 28/3/20.
//

#ifndef ANIMESH_CROSSFIELDGLCANVAS_H
#define ANIMESH_CROSSFIELDGLCANVAS_H

#include <nanogui/nanogui.h>
#include <Camera/Camera.h>
#include "types.h"

class CrossFieldGLCanvas : public nanogui::GLCanvas {
public:
    CrossFieldGLCanvas(Widget *parent);

    ~CrossFieldGLCanvas();

    virtual void drawGL() override;

    void update_mvp();
    void setAzimuth(float azimuth ) {
        m_azimuth = azimuth;
        update_mvp();
    }
    void setFrame(int frame ) {
        m_frame_idx = frame;
        load_gl_data();
    }
    void setRadius(float radius) {
        m_radius = radius;
        update_mvp();
    }

    void set_data(const std::vector<std::vector<nanogui::Vector3f>>& points,
                                      const std::vector<std::vector<nanogui::Vector3f>>& normals,
                                      const std::vector<std::vector<nanogui::Vector3f>>& tangents);

private:
    int m_num_surfels;
    nanogui::GLShader m_shader;
    float m_azimuth = 0.0f;
    float m_radius = 0.0f;

    std::vector<std::vector<nanogui::Vector3f>> m_points;
    std::vector<std::vector<nanogui::Vector3f>> m_normals;
    std::vector<std::vector<nanogui::Vector3f>> m_tangents;


    nanogui::Vector3f m_centroid;

    size_t m_frame_idx = 0;
    nanogui::Matrix4f m_projection;
    nanogui::Matrix4f m_mvp;
    nanogui::MatrixXf make_vertices(
            const std::vector<std::vector<nanogui::Vector3f>> &points,
            const std::vector<std::vector<nanogui::Vector3f>> &normals,
            const std::vector<std::vector<nanogui::Vector3f>> &tangents);
    void load_gl_data( );
    void make_projection( );
};


#endif //ANIMESH_CROSSFIELDGLCANVAS_H
