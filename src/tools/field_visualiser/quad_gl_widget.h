//
// Created by Dave Durbin (Old) on 28/6/21.
//

#pragma once


#include <QOpenGLWidget>
#include <ArcBall/ArcBall.h>
#include "field_gl_widget.h"

class quad_gl_widget : public field_gl_widget {
Q_OBJECT
public:
    explicit quad_gl_widget(
            QWidget *parent = nullptr,
            Qt::WindowFlags f = Qt::WindowFlags());

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
    void do_paint() override;

    void initializeGL() override;

private:
    bool m_show_blue_edges;
    bool m_show_red_edges;

    std::vector<float> m_vertices;
    std::vector<std::pair<unsigned int, unsigned int>> m_red_edges;
    std::vector<std::pair<unsigned int, unsigned int>> m_blue_edges;

    void drawVertices() const;
    void maybe_draw_red_edges() const;
    void maybe_draw_blue_edges() const;

    void mouse_moved(unsigned int pixel_x, unsigned int pixel_y);
};