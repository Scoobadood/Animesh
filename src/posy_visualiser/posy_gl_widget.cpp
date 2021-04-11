#include "posy_gl_widget.h"

#include <vector>
#include <QColor>
#include <Surfel/SurfelGraph.h>

const float DEG2RAD = (3.14159265f / 180.0f);

posy_gl_widget::posy_gl_widget(QWidget *parent, Qt::WindowFlags f) :
        QOpenGLWidget{parent, f} //
        , m_fov{60} //
        , m_zNear{0.5f} //
        , m_zFar{50.0f} //
        , m_aspectRatio{1.0f} //
        , m_projectionMatrixIsDirty{true} //
        , m_normalScaleFactor{1.0f} //
{
    m_arcBall = new ArcBall();
    installEventFilter(m_arcBall);
    setFocus();
    // Dummy data
    setPoSyData(
            std::vector<float>{0.0f, 0.0f, 0.0f},
            std::vector<float>{0.0f, 1.0f, 0.0f},
            std::vector<float>{0.0f, 0.0f},
            1.0f
    );
}

void
posy_gl_widget::drawPositions() const {
    glColor4d(1.0, 1.0, 1.0, 1.0);

    glEnable(GL_POINT_SMOOTH);
    float oldPointSize;
    glGetFloatv(GL_POINT_SIZE, &oldPointSize);

    glPointSize(3.0f);
    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        glBegin(GL_POINTS);
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glEnd();
    }
    glPointSize(oldPointSize);
    glDisable(GL_POINT_SMOOTH);
    checkGLError("drawPositions");
}

void
posy_gl_widget::clear() {
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGLError("clear");
}

/**
 * If the ModelView matrix is dirty, update and reload it.
 */
void
posy_gl_widget::maybeUpdateModelViewMatrix() {
    if( m_arcBall->modelViewMatrixHasChanged()) {
        glMatrixMode(GL_MODELVIEW);
        float m[16];
        m_arcBall->modelViewMatrix(m);
        glLoadMatrixf(m);
    }
    checkGLError("maybeUpdateModelViewMatrix");
}

/**
 * If the projection matrix is dirty, update and reload it.
 */
void
posy_gl_widget::maybeUpdateProjectionMatrix() const {
    if (m_projectionMatrixIsDirty) {
        const auto yMax = tan(m_fov * DEG2RAD * 0.5f);
        const auto xMax = yMax * m_aspectRatio;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-xMax, xMax, -yMax, yMax, m_zNear, m_zFar);
    }
    checkGLError("maybeUpdateProjectionMatrix");
}

/**
 * Resize the viewport when the window resizes.
 */
void
posy_gl_widget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    m_aspectRatio = (float) width / (float) height;
    m_projectionMatrixIsDirty = true;
}

void
posy_gl_widget::paintGL() {
    clear();

    maybeUpdateProjectionMatrix();

    maybeUpdateModelViewMatrix();

    drawPositions();
}

void
posy_gl_widget::setPoSyData(const std::vector<float> &positions,
                            const std::vector<float> &normals,
                            const std::vector<float> &uvs,
                            const float normal_scale_factor) {
    m_positions.clear();
    m_normals.clear();
    m_uvs.clear();

    m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
    m_normals.insert(m_normals.begin(), normals.begin(), normals.end());
    m_uvs.insert(m_uvs.begin(), uvs.begin(), uvs.end());
    m_normalScaleFactor = normal_scale_factor;

    update();
}

void
posy_gl_widget::checkGLError(const std::string &context) {
    auto err = glGetError();
    if (!err) return;
    spdlog::error("{}: {} ", context, err);
}

void
posy_gl_widget::initializeGL() {
    glEnable(GL_DEPTH);
}

void
posy_gl_widget::setZFar(float zFar) {
    if (m_zFar != zFar) {
        m_zFar = zFar;
        m_projectionMatrixIsDirty = true;
        update();
    }
}

void
posy_gl_widget::setFov(float fov) {
    if( m_fov != fov) {
        m_fov = fov;
        m_projectionMatrixIsDirty = true;
        update();
    }
}