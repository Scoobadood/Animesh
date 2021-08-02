//
// Created by Dave Durbin (Old) on 28/6/21.
//

#pragma once


#include <QOpenGLWidget>
#include <ArcBall/ArcBall.h>
#include "gl_tools.h"

class quad_gl_widget : public QOpenGLWidget {
Q_OBJECT
public:
    explicit quad_gl_widget(
            QWidget *parent = nullptr,
            Qt::WindowFlags f = Qt::WindowFlags());

    void set_arc_ball(ArcBall *arc_ball);
    void setData(
            const std::vector<float> &vertices,
            const std::vector<std::pair<unsigned int, unsigned int>> &red_edges,
            const std::vector<std::pair<unsigned int, unsigned int>> &blue_edges);

    inline void showBlueEdges(bool should_show) {
        if( m_show_blue_edges != should_show) {
            m_show_blue_edges = should_show;
            update();
        }
    }
    inline void showRedEdges(bool should_show) {
        if( m_show_red_edges != should_show) {
            m_show_red_edges = should_show;
            update();
        }
    }
protected:
    void paintGL() override;

    void do_paint();

    void initializeGL() override;

    void resizeGL(int width, int height) override;

    static void checkGLError(const std::string &context);

    static void clear();

private:
    ArcBall *m_arcBall;

    float m_fov;
    float m_zNear;
    float m_zFar;
    float m_aspectRatio;
    bool m_projectionMatrixIsDirty;
    bool m_show_blue_edges;
    bool m_show_red_edges;

    void update_model_matrix();

    void maybe_update_projection_matrix() const;

    std::vector<float> m_vertices;
    std::vector<std::pair<unsigned int, unsigned int>> m_red_edges;
    std::vector<std::pair<unsigned int, unsigned int>> m_blue_edges;

    void drawVertices() const;
    void maybe_draw_red_edges() const;
    void maybe_draw_blue_edges() const;

    void mouse_moved();
};