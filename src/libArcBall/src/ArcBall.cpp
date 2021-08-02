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
ArcBall::get_model_view_matrix(float *mat) {
    if( m_modelViewMatrixIsDirty) {
        m_model_view_matrix.setToIdentity();

        const auto pos = getCameraPosition();
        m_model_view_matrix.lookAt(
                pos,
                m_target,
                QVector3D(0.0f, m_up, 0.0f));
        m_model_view_matrix.translate(-pos.x(), -pos.y(), -pos.z());
        m_modelViewMatrixIsDirty = false;
    }
    memcpy(mat, m_model_view_matrix.constData(), 16 * sizeof(float));
}

QVector3D
ArcBall::compute_ray_through_pixel(unsigned int pixel_x,
                                   unsigned int pixel_y,
                                   QVector2D field_of_view,
                                   float focal_length,
                                   int width,
                                   int height) {
  using namespace Eigen;

  const auto camera_origin = getCameraPosition();

  auto N = camera_origin-m_target;
  const auto n = N.normalized();

  // u is a vector that is perpendicular to the plane spanned by
  // N and view up vector (cam->up), ie in the image plane and horizontal
  auto U = QVector3D::crossProduct(QVector3D{0, m_up, 0}, n);
  const auto u = U.normalized();

  // v is a vector perpendicular to N and U, i.e vertical in image palne
  const auto v = QVector3D::crossProduct(n,u);

  const auto fov_rad = field_of_view * DEG2RAD;
  double image_plane_height = 2 * tan(fov_rad.y() * 0.5f) * focal_length;
  double image_plane_width = 2 * tan(fov_rad.x() * 0.5f) * focal_length;

  const auto image_plane_centre = camera_origin - (n * focal_length);
  const auto image_plane_origin = image_plane_centre - (u * image_plane_width * 0.5f) - (v * image_plane_height * 0.5f);

  // Compute pixel dimensions in world units
  const auto pixel_width = image_plane_width / width;
  const auto pixel_height = image_plane_height / height;

  // Construct a ray from the camera origin to the world coordinate
  auto pixel_in_world = image_plane_origin
      + ((pixel_x + 0.5) * pixel_width * u)
      + ((pixel_y + 0.5) * pixel_height * v);

  return  (pixel_in_world - camera_origin).normalized();
}