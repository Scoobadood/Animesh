#include "rosy_gl_widget.h"

#include <iostream>
#include <vector>
#include <QColor>
#include <QKeyEvent>
#include <QQuaternion>
#include <Geom/Geom.h>
#include <Surfel/SurfelGraph.h>

const float DEG2RAD = (3.14159265f / 180.0f);
const float TWO_PI = M_PI * 2.0f;

rosy_gl_widget::rosy_gl_widget(QWidget *parent, Qt::WindowFlags f) :
        QOpenGLWidget{parent, f}, m_normalScaleFactor{1.0f} //
        , m_renderNormals{true} //
        , m_renderMainTangents{true} //
        , m_renderOtherTangents{true} //
        , m_normalColour{QColor{255, 255, 255, 255}} //
        , m_mainTangentColour{QColor{255, 0, 0, 255}} //
        , m_otherTangentsColour{QColor{0, 255, 0, 255}} //
        , m_fov{60} //
        , m_zNear{0.5f} //
        , m_zFar{50.0f} //
        , m_aspectRatio{1.0f} //
        , m_theta{0.0f} //
        , m_phi{0.0f} //
        , m_radius{1.0f} //
        , m_up{1.0f} //
        , m_modelViewMatrixIsDirty{true} //
        , m_projectionMatrixIsDirty{true} //
        , m_target{0.0f, 0.0f, 0.0f} //
{
    setFocus();
    // Dummy data
    setRoSyData(
            std::vector<float>{0.0f, 0.0f, 0.0f},
            std::vector<float>{0.0f, 1.0f, 0.0f},
            std::vector<float>{0.0f, 0.0f, 1.0f},
            1.0f
    );
}

void
rosy_gl_widget::maybeDrawNormals() const {
    if (!m_renderNormals) {
        return;
    }
    glColor4d(m_normalColour.redF(), m_normalColour.greenF(),
              m_normalColour.blueF(), m_normalColour.alphaF());

    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        glBegin(GL_LINES);
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glVertex3f(m_positions.at(i * 3 + 0) + (m_normals.at(i * 3 + 0) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 1) + (m_normals.at(i * 3 + 1) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 2) + (m_normals.at(i * 3 + 2) * m_normalScaleFactor));
        glEnd();
    }
    checkGLError("maybeDrawNormals");

}

void
rosy_gl_widget::drawPositions() const {
    glColor4d(m_normalColour.redF(),m_normalColour.greenF(),
              m_normalColour.blueF(),m_normalColour.alphaF());

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
rosy_gl_widget::maybeDrawMainTangents() const {
    if (!m_renderMainTangents) {
        return;
    }
    glColor4d(m_mainTangentColour.redF(),m_mainTangentColour.greenF(),
              m_mainTangentColour.blueF(),m_mainTangentColour.alphaF());

    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        glBegin(GL_LINES);
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glVertex3f(m_positions.at(i * 3 + 0) + (m_tangents.at(i * 3 + 0) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 1) + (m_tangents.at(i * 3 + 1) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 2) + (m_tangents.at(i * 3 + 2) * m_normalScaleFactor));
        glEnd();
    }
    checkGLError("maybeDrawTangents");

}

void
rosy_gl_widget::maybeDrawOtherTangents() const {
    if (!m_renderOtherTangents) {
        return;
    }
    glColor4f((GLfloat) m_otherTangentsColour.redF(),
              (GLfloat) m_otherTangentsColour.greenF(),
              (GLfloat) m_otherTangentsColour.blueF(),
              (GLfloat) m_otherTangentsColour.alphaF());

    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        // Get perpendicular tangent by computing cross(norm,tan)
        const auto normX = m_normals.at(i * 3 + 0);
        const auto normY = m_normals.at(i * 3 + 1);
        const auto normZ = m_normals.at(i * 3 + 2);
        const auto tanX = m_tangents.at(i * 3 + 0);
        const auto tanY = m_tangents.at(i * 3 + 1);
        const auto tanZ = m_tangents.at(i * 3 + 2);

        auto crossTanX = (normY * tanZ - normZ * tanY) * m_normalScaleFactor; //bn -cm
        auto crossTanY = (normZ * tanX - normX * tanZ) * m_normalScaleFactor; //bn -cm
        auto crossTanZ = (normX * tanY - normY * tanX) * m_normalScaleFactor; //bn -cm

        glBegin(GL_LINES);
        glVertex3f(m_positions.at(i * 3 + 0) - crossTanX,
                   m_positions.at(i * 3 + 1) - crossTanY,
                   m_positions.at(i * 3 + 2) - crossTanZ);
        glVertex3f(m_positions.at(i * 3 + 0) + crossTanX,
                   m_positions.at(i * 3 + 1) + crossTanY,
                   m_positions.at(i * 3 + 2) + crossTanZ);
        glVertex3f(m_positions.at(i * 3 + 0) - (tanX * m_normalScaleFactor),
                   m_positions.at(i * 3 + 1) - (tanY * m_normalScaleFactor),
                   m_positions.at(i * 3 + 2) - (tanZ * m_normalScaleFactor));
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glEnd();
    }
    checkGLError("maybeDrawOtherTangents");
}

void
rosy_gl_widget::clear() {
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGLError("clear");
}

/**
 * If the ModelView matrix is dirty, update and reload it.
 */
void
rosy_gl_widget::maybeUpdateModelViewMatrix() {
    if (m_modelViewMatrixIsDirty) {
        QMatrix4x4 matrix;
        matrix.setToIdentity();

        const auto pos = getCameraPosition();
        matrix.lookAt(
                pos,
                m_target,
                QVector3D(0.0f, m_up, 0.0f));
        matrix.translate(-pos.x(), -pos.y(), -pos.z());
        m_modelViewMatrixIsDirty = false;

        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(matrix.data());
    }
    checkGLError("maybeUpdateModelViewMatrix");
}

/**
 * If the projection matrix is dirty, update and reload it.
 */
void
rosy_gl_widget::maybeUpdateProjectionMatrix() const {
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
rosy_gl_widget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    m_aspectRatio = (float) width / (float) height;
    m_projectionMatrixIsDirty = true;
}

void
rosy_gl_widget::paintGL() {
    clear();

    maybeUpdateProjectionMatrix();

    maybeUpdateModelViewMatrix();

    drawPositions();

    maybeDrawNormals();

    maybeDrawMainTangents();

    maybeDrawOtherTangents();
}

void
rosy_gl_widget::setRoSyData(const std::vector<float> &positions,
                            const std::vector<float> &normals,
                            const std::vector<float> &tangents,
                            const float normal_scale_factor) {
    m_positions.clear();
    m_tangents.clear();
    m_normals.clear();

    m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
    m_tangents.insert(m_tangents.begin(), tangents.begin(), tangents.end());
    m_normals.insert(m_normals.begin(), normals.begin(), normals.end());
    m_normalScaleFactor = normal_scale_factor;

    update();
}

void
rosy_gl_widget::checkGLError(const std::string &context) {
    auto err = glGetError();
    if (!err) return;
    spdlog::error("{}: {} ", context, err);
}

void
rosy_gl_widget::initializeGL() {
    glEnable(GL_DEPTH);
}

void rosy_gl_widget::keyPressEvent(QKeyEvent *event) {
    bool shiftDown = (event->modifiers() & Qt::KeyboardModifier::ShiftModifier) == Qt::KeyboardModifier::ShiftModifier;
    bool movedCamera = false;
    bool rotatedCamera = false;
    switch (event->key()) {
        case Qt::Key_Left:
            if (shiftDown) {
                rotate(-10.0f * DEG2RAD, 0);
                rotatedCamera = true;
            } else {
                pan(-0.1f, 0.0f);
                movedCamera = true;
            }
            break;

        case Qt::Key_Right:
            if (shiftDown) {
                rotate(10.0f * DEG2RAD, 0);
                rotatedCamera = true;
            } else {
                pan(0.1f, 0.0f);
                movedCamera = true;
            }
            break;

        case Qt::Key_Up:
            if (shiftDown) {
                rotate(0.0f, -10.f * DEG2RAD);
                rotatedCamera = true;
            } else {
                pan(0.0f, -0.1f);
                movedCamera = true;
            }
            break;

        case Qt::Key_Down:
            if (shiftDown) {
                rotate(0.0f, 10.f * DEG2RAD);
                rotatedCamera = true;
            } else {
                pan(0.0f, 0.1f);
                movedCamera = true;
            }
            break;

        case Qt::Key_Equal:
            zoom(-0.5f);
            movedCamera = true;
            break;

        case Qt::Key_Minus:
            zoom(0.5f);
            movedCamera = true;
            break;

        default:
            std::cout << "Key pressed " << event->key() << std::endl;
    }
    if (movedCamera || rotatedCamera) {
        if (movedCamera) {
            const auto pos = toCartesian();
            emit cameraPositionChanged(pos.x(), pos.y(), pos.z());
        }
        if (rotatedCamera) {
            emit cameraOrientationChanged(m_radius, m_theta, m_phi);
        }
        m_modelViewMatrixIsDirty = true;
        update();
    }
}

void
rosy_gl_widget::mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() & Qt::LeftButton) {
        if (e->modifiers() & Qt::KeyboardModifier::ControlModifier) {
            const auto delta = m_lastPixelPosition - e->pos();
            pan((float) delta.x() * PAN_FACTOR, (float) delta.y() * PAN_FACTOR);
        } else {
            const auto delta = (m_lastPixelPosition - e->pos());
            rotate(-(float) delta.x() / ROTATE_FACTOR, (float) delta.y() / ROTATE_FACTOR);
        }
    } else if (e->buttons() & Qt::RightButton) {
        const auto zDelta = (m_lastPixelPosition - e->pos()).y();
        zoom((float) zDelta * ZOOM_FACTOR);
    }
    m_lastPixelPosition = e->pos();
    e->accept();
}

void
rosy_gl_widget::mousePressEvent(QMouseEvent *e) {
    if (e->buttons() & (Qt::LeftButton | Qt::RightButton)) {
        m_lastPixelPosition = e->pos();
    }
    e->accept();
}

void
rosy_gl_widget::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();
}

void
rosy_gl_widget::rotate(float dTheta, float dPhi) {
    if (m_up > 0.0f) {
        m_theta += dTheta;
    } else {
        m_theta -= dTheta;
    }

    m_phi += dPhi;

    // Keep phi within -2PI to +2PI for easy 'up' comparison
    if (m_phi > TWO_PI) {
        m_phi -= TWO_PI;
    } else if (m_phi < -TWO_PI) {
        m_phi += TWO_PI;
    }

    // If phi is between 0 to PI or -PI to -2PI, make 'up' be positive Y, other wise make it negative Y
    if ((m_phi > 0 && m_phi < M_PI) || (m_phi < -M_PI && m_phi > -TWO_PI)) {
        m_up = 1.0f;
    } else {
        m_up = -1.0f;
    }
    m_modelViewMatrixIsDirty = true;
    emit cameraOrientationChanged(m_radius, m_theta, m_phi);
    update();
}

void
rosy_gl_widget::zoom(float distance) {
    m_radius = std::fminf(30.0f, std::fmaxf(0.0f, m_radius - distance));
    m_modelViewMatrixIsDirty = true;
    emit cameraOrientationChanged(m_radius, m_theta, m_phi);
    update();
}

void
rosy_gl_widget::pan(float dx, float dy) {
    const auto lookDirection = (m_target - getCameraPosition()).normalized();
    const auto worldUp = QVector3D(0.0f, m_up, 0.0f);
    const auto right = QVector3D::crossProduct(lookDirection, worldUp);
    const auto up = QVector3D::crossProduct(lookDirection, right);

    m_target = m_target + (right * dx) + (up * dy);
    m_modelViewMatrixIsDirty = true;
    update();
}

QVector3D
rosy_gl_widget::getCameraPosition() const {
    return m_target + toCartesian();
}

QVector3D
rosy_gl_widget::toCartesian() const {
    const auto vector = spherical_to_cartesian(m_radius, m_theta, m_phi);
    emit cameraPositionChanged(vector.x(), vector.y(), vector.z());
    return {vector.x(), vector.y(), vector.z()};
}

void
rosy_gl_widget::renderNormals( bool shouldRender) {
    if( m_renderNormals != shouldRender) {
        m_renderNormals = shouldRender;
        update();
    }
}

void
rosy_gl_widget::renderMainTangents( bool shouldRender) {
    if( m_renderMainTangents != shouldRender) {
        m_renderMainTangents = shouldRender;
        update();
    }
}

void
rosy_gl_widget::renderOtherTangents( bool shouldRender) {
    if( m_renderOtherTangents != shouldRender) {
        m_renderOtherTangents = shouldRender;
        update();
    }
}

void
rosy_gl_widget::setZFar(float zFar) {
    if (m_zFar != zFar) {
        m_zFar = zFar;
        m_projectionMatrixIsDirty = true;
        update();
    }
}

void
rosy_gl_widget::setFov(float fov) {
    if( m_fov != fov) {
        m_fov = fov;
        m_projectionMatrixIsDirty = true;
        update();
    }
}