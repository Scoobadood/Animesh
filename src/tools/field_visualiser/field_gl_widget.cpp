//
// Created by Dave Durbin (Old) on 28/6/21.
//

#include <spdlog/spdlog.h>
#include "field_gl_widget.h"

const float DEG2RAD = (3.14159265f / 180.0f);

field_gl_widget::field_gl_widget(
        QWidget *parent, Qt::WindowFlags f) :
        QOpenGLWidget{parent, f} //
        , m_fov{35} //
        , m_zNear{0.5f} //
        , m_zFar{1000.0f} //
        , m_aspectRatio{1.0f} //
        , m_projectionMatrixIsDirty{true} //
{
    setFocus();
}

void
field_gl_widget::set_arc_ball(ArcBall *arc_ball) {
    m_arcBall = arc_ball;
    installEventFilter(arc_ball);
}

void
field_gl_widget::clear() {
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGLError("clear");
}

/**
 * If the ModelView matrix is dirty, update and reload it.
 */
void
field_gl_widget::update_model_matrix() {
    glMatrixMode(GL_MODELVIEW);
    float m[16];
    m_arcBall->get_model_view_matrix(m);
    glLoadMatrixf(m);
    checkGLError("update_model_matrix");
}

/**
 * If the projection matrix is dirty, update and reload it.
 */
void
field_gl_widget::maybe_update_projection_matrix() const {
    if (m_projectionMatrixIsDirty) {
        const auto yMax = tan(m_fov * DEG2RAD * 0.5f);
        const auto xMax = yMax * m_aspectRatio;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-xMax, xMax, -yMax, yMax, m_zNear, m_zFar);
    }
    checkGLError("maybe_update_projection_matrix");
}

void
field_gl_widget::paintGL() {
    clear();

    maybe_update_projection_matrix();

    update_model_matrix();

    do_paint();
}

/**
 * Resize the viewport when the window resizes.
 */
void
field_gl_widget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    m_aspectRatio = (float) width / (float) height;
    m_projectionMatrixIsDirty = true;
}

void
field_gl_widget::checkGLError(const std::string &context) {
    auto err = glGetError();
    if (!err) return;
    spdlog::error("{}: {} ", context, err);
}
