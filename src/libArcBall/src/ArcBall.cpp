//
// Created by Dave Durbin (Old) on 10/4/21.
//

#include "ArcBall.h"
#include <Geom/Geom.h>
#include <cmath>

#include <QMouseEvent>
#include <QMatrix4x4>
#include <QOpenGLWidget>

const float DEG2RAD = (3.14159265f / 180.0f);
const float TWO_PI = M_PI * 2.0f;

ArcBall::ArcBall() :
        m_theta{0.0f} //
        , m_phi{0.0f} //
        , m_radius{1.0f} //
        , m_up{1.0f} //
        , m_modelViewMatrixIsDirty{true} //
        , m_target{0.0f, 0.0f, 0.0f} //
{}

void
ArcBall::rotate(float dTheta, float dPhi) {
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
}

void
ArcBall::zoom(float distance) {
    m_radius = std::fminf(30.0f, std::fmaxf(0.0f, m_radius - distance));
    m_modelViewMatrixIsDirty = true;
}

void
ArcBall::pan(float dx, float dy) {
    const auto lookDirection = (m_target - getCameraPosition()).normalized();
    const auto worldUp = QVector3D(0.0f, m_up, 0.0f);
    const auto right = QVector3D::crossProduct(lookDirection, worldUp);
    const auto up = QVector3D::crossProduct(lookDirection, right);

    m_target = m_target + (right * dx) + (up * dy);
    m_modelViewMatrixIsDirty = true;
}

QVector3D
ArcBall::getCameraPosition() const {
    return m_target + toCartesian();
}

QVector3D
ArcBall::toCartesian() const {
    const auto vector = spherical_to_cartesian(m_radius, m_theta, m_phi);
    return {vector.x(), vector.y(), vector.z()};
}

void
ArcBall::mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() & Qt::LeftButton) {
        if (e->modifiers() & Qt::KeyboardModifier::ControlModifier) {
            const auto delta = m_lastPixelPosition - e->pos();
            pan((float) delta.x() * PAN_FACTOR, (float) delta.y() * PAN_FACTOR);
        } else {
            const auto delta = (m_lastPixelPosition - e->pos());
            rotate((float) delta.x() / ROTATE_FACTOR, (float) delta.y() / ROTATE_FACTOR);
        }
    }
    m_lastPixelPosition = e->pos();
    e->accept();
}

void
ArcBall::wheelEvent(QWheelEvent *e) {
    const auto zDelta = e->angleDelta().y();
    zoom((float) zDelta * ZOOM_FACTOR);
}

void
ArcBall::mousePressEvent(QMouseEvent *e) {
    if (e->buttons() & (Qt::LeftButton | Qt::RightButton)) {
        m_lastPixelPosition = e->pos();
    }
    e->accept();
}

void
ArcBall::keyPressEvent(QKeyEvent *event) {
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
    }
    if (movedCamera || rotatedCamera) {
        m_modelViewMatrixIsDirty = true;
    }
}

bool
ArcBall::eventFilter(QObject *o, QEvent *e) {
    bool handled;
    switch (e->type()) {
        case QEvent::Wheel:
            wheelEvent((QWheelEvent *)e);
            handled = true;
            break;

        case QEvent::MouseMove:
            mouseMoveEvent((QMouseEvent *) e);
            handled = true;
            break;

        case QEvent::MouseButtonPress:
            mousePressEvent((QMouseEvent *) e);
            handled = true;
            break;

        case QEvent::KeyPress:
            keyPressEvent((QKeyEvent *) e);
            handled = true;
            break;

        default:
            handled = false;
            break;
    }
    if( handled) {
        ((QOpenGLWidget *) o)->update();
    }
    return handled;
}

void
ArcBall::modelViewMatrix(float mat[16]) {
    QMatrix4x4 matrix;
    matrix.setToIdentity();

    const auto pos = getCameraPosition();
    matrix.lookAt(
            pos,
            m_target,
            QVector3D(0.0f, m_up, 0.0f));
    matrix.translate(-pos.x(), -pos.y(), -pos.z());
    memcpy(mat, matrix.constData(), 16 * 4);
    m_modelViewMatrixIsDirty = false;
}