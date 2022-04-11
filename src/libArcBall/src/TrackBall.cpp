
#include <ArcBall/TrackBall.h>
#include <QVector3D>
#include <QVector2D>
#include <QQuaternion>
#include <QMouseEvent>
#include <QOpenGLWidget>

#define   FloatInfinity std::numeric_limits<float>::infinity()
#define   SQRT1_2  0.7071067811865476

Trackball::Trackball(
    const QVector3D &cam_pos,
    int screen_x,
    int screen_y,
    int screen_width,
    int screen_height)
    : m_mode(MODE::NONE) //
    , m_screen_origin_x(screen_x) //
    , m_screen_origin_y(screen_y) //
    , m_screen_width(screen_width) //
    , m_screen_height(screen_height) //
    , m_cam_location(cam_pos) //
    , m_cam_up(0.0f, 1.0f, 0.0f) //
    , m_cam_view_matrix{} //
    , m_cam_view_matrix_is_dirty{true} //
    , m_focal_point(0, 0, 0) //
    , m_target_to_cam(0, 0, 0) //
    , m_zoom_mouse_start(0, 0) //
    , m_zoom_mouse_end(0, 0) //
    , m_zoom_speed(1.2f) //
    , m_zoom_enabled(true) //
    , m_rotate_mouse_start(0, 0, 0) //
    , m_rotate_mouse_end(0, 0, 0) //
    , m_rotate_speed(9.0f) //
    , m_rotation_enabled(true) //
    , m_pan_mouse_start(0, 0) //
    , m_pan_mouse_end(0, 0) //
    , m_pan_speed(0.3f) //
    , m_pan_enabled(true) //
    , m_damping_factor(0.2f) //
    , m_minDistance(0.0f) //
    , m_maxDistance(FloatInfinity) //
    , m_enabled(true) //
    , m_no_roll(false) //
    , m_static_moving(true) //
{}

void Trackball::reset() {
  m_cam_up = {0.0f, 1.0f, 0.0f};
  m_cam_location = {10, 0, 0};
  m_cam_view_matrix_is_dirty = true;
}

void
Trackball::get_model_view_matrix(float mat[16]) {
  maybe_update_matrix();

  memcpy(mat, m_cam_view_matrix.constData(), 16 * sizeof(float));
}

QVector3D
Trackball::get_camera_origin() const {
  return {
      m_cam_location.x(), m_cam_location.y(), m_cam_location.z()
  };
}

bool
Trackball::eventFilter(QObject *o, QEvent *e) {
  bool handled;
  switch (e->type()) {
  case QEvent::Wheel: {//
    auto we = (QWheelEvent *) e;
    mouse_wheel(we->position().x(), we->position().y());
    handled = true;
  }
    break;

  case QEvent::MouseMove: {//
    auto me = (QMouseEvent *) e;
    mouse_move(me->x(), me->y());
    m_mouse_moved = true;
    handled = e->isAccepted();
  }
    break;

  case QEvent::MouseButtonPress: {//
    auto me = (QMouseEvent *) e;
    mouse_down(me->button(), me->modifiers(), me->pos().x(), me->pos().y());
    handled = false;
    m_mouse_moved = false;
  }
    break;

  case QEvent::MouseButtonRelease: {
    mouse_up();
    return m_mouse_moved;
  }
    break;

  case QEvent::KeyPress: {//
    auto ke = (QKeyEvent *) e;
    key_down(ke->key());
    handled = true;
  }
    break;

  case QEvent::KeyRelease: {//
    key_up();
    handled = true;
  }
    break;

  case QEvent::Resize: {//
    auto re = (QResizeEvent *) e;
    m_screen_width = re->size().width();
    m_screen_height = re->size().height();
    handled = false;
  }
    break;

  default:handled = false;
    break;
  }
  if (handled) {
    ((QOpenGLWidget *) o)->update();
  }
  return handled;
}

void Trackball::maybe_update_matrix() {
  if (!m_cam_view_matrix_is_dirty) {
    return;
  }

  m_target_to_cam = m_cam_location - m_focal_point;

  maybe_update_rotation();
  maybe_update_zoom();
  maybe_update_pan();

  m_cam_location = m_focal_point + m_target_to_cam;

  CheckDistances();

  m_cam_view_matrix.setToIdentity();
  m_cam_view_matrix.lookAt(m_cam_location, m_focal_point, m_cam_up);

  m_cam_view_matrix_is_dirty = false;
}

void Trackball::maybe_update_rotation() {
  if (!m_rotation_enabled) {
    return;
  }

  auto cos_alpha = QVector3D::dotProduct(m_rotate_mouse_start / m_rotate_mouse_start.length(),
                                         m_rotate_mouse_end / m_rotate_mouse_end.length());
  auto angle = acosf(std::fminf(1.0f, std::fmaxf(-1.0, cos_alpha)));
  auto axis = (QVector3D::crossProduct(m_rotate_mouse_start, m_rotate_mouse_end)).normalized();
  angle *= m_rotate_speed;
  QQuaternion quaternion = QQuaternion::fromAxisAndAngle(axis, -angle);
  m_target_to_cam = quaternion.rotatedVector(m_target_to_cam);
  m_cam_up = quaternion.rotatedVector(m_cam_up);
  m_rotate_mouse_end = quaternion.rotatedVector(m_rotate_mouse_end);
}

void Trackball::maybe_update_zoom() {
  if (!m_zoom_enabled) {
    return;
  }

  auto factor = 1.0f + ((m_zoom_mouse_end.y() - m_zoom_mouse_start.y()) * m_zoom_speed);
  if (factor != 1.0f && factor > 0.0f) {
    m_target_to_cam = m_target_to_cam * factor;
    if (m_static_moving) {
      m_zoom_mouse_start = m_zoom_mouse_end;
    } else {
      auto delta_y = (m_zoom_mouse_end.y() - m_zoom_mouse_start.y()) * m_damping_factor;
      m_zoom_mouse_start.setY(m_zoom_mouse_start.y() + delta_y);
    }
  }
}

void Trackball::maybe_update_pan() {
  if (!m_pan_enabled) {
    return;
  }

  QVector2D pan_delta = m_pan_mouse_end - m_pan_mouse_start;
  // If we moved significantly
  if (pan_delta.lengthSquared() > 0) {
    pan_delta *= (m_target_to_cam.length() * m_pan_speed);

    // Compute tangent vector to trackball
    auto pan_x_direction = QVector3D::crossProduct(m_target_to_cam, m_cam_up).normalized();
    auto pan_y_direction = m_cam_up;
    QVector3D pan{0, 0, 0};
    pan += (pan_delta.x() * pan_x_direction);
    pan += (pan_delta.y() * pan_y_direction);
    m_cam_location += pan;
    m_focal_point += pan;

    if (m_static_moving) {
      m_pan_mouse_start = m_pan_mouse_end;
    } else {
      m_pan_mouse_start += (pan_delta * m_damping_factor);
    }
  }
}

void Trackball::CheckDistances() {
  if (m_zoom_enabled || m_pan_enabled) {
    if (m_cam_location.lengthSquared() > (m_maxDistance * m_maxDistance)) {
      m_cam_location = m_cam_location.normalized() * m_maxDistance;
    }
    if (m_target_to_cam.lengthSquared() < (m_minDistance * m_minDistance)) {
      m_target_to_cam = m_target_to_cam.normalized() * m_minDistance;
      m_cam_location = m_focal_point + m_target_to_cam;
    }
  }
}

/**
 * Assuming a unit sphere centred on the target and that the Z axis runs from cam
 * through target, compute the projection of the mouse coordinates into
 * a point on that sphere.
 */
QVector3D
Trackball::get_mouse_projection_on_ball(int mouse_x, int mouse_y) {
  auto half_width = static_cast<float>(m_screen_width) * 0.5f;
  auto half_height = static_cast<float>(m_screen_height) * 0.5f;

  // Assuming the trackball is a unit sphere centred at 0,0,0
  // Compute the XY coords of the mouse click on Z plane (X and Y are in range [-1,1])
  QVector3D mouse_on_ball{
      (static_cast<float>(mouse_x) - half_width) / half_width,
      (half_height - static_cast<float>(mouse_y)) / half_height,
      0.0f
  };

  auto length = mouse_on_ball.length();
  if (m_no_roll) {
    //
    if (length < SQRT1_2) {
      mouse_on_ball.setZ(std::sqrtf(1.0f - length * length));
    } else {
      mouse_on_ball.setZ(0.5f / length);
    }
  }
    // If the XY is outside the sphere, move it onto the sphere at Z = 0
  else if (length > 1.0) {
    mouse_on_ball.normalize();
  }
    // Otherwise XY is 'inside' the projection of the circumference on screen,
    // set Z on front face of sphere
  else {
    mouse_on_ball.setZ(std::sqrtf(1.0f - length * length));
  }

//  m_target_to_cam = m_focal_point - m_cam_location;

  // Y component of
  auto unit_y = m_cam_up.normalized();
  auto unit_x = QVector3D::crossProduct(m_cam_up, m_target_to_cam).normalized();
  auto unit_z = m_target_to_cam.normalized(); // We're looking down the Z axis

  QVector3D projection =
      (mouse_on_ball.x() * unit_x) +
          (mouse_on_ball.y() * unit_y) +
          (mouse_on_ball.z() * unit_z);

  return projection;
}

void
Trackball::mouse_down(Qt::MouseButton button, Qt::KeyboardModifiers mods, int xpos, int ypos) {
  if (!m_enabled) {
    return;
  }

  if (m_mode == MODE::NONE) {
    if (mods.testFlag(Qt::KeyboardModifier::MetaModifier)) {
      if (m_pan_enabled) {
        m_mode = MODE::PAN;
      }
    } else if (m_rotation_enabled) {
      m_mode = MODE::ROTATE;
    }
  }

  switch (m_mode) {
  case MODE::NONE: return;
  case MODE::PAN:m_pan_mouse_start = normalize_mouse_coords(xpos, ypos);
    m_pan_mouse_end = m_pan_mouse_start;
    break;
  case MODE::ZOOM:m_zoom_mouse_start = normalize_mouse_coords(xpos, ypos);
    m_zoom_mouse_end = m_zoom_mouse_start;
    break;
  case MODE::ROTATE:m_rotate_mouse_start = get_mouse_projection_on_ball(xpos, ypos);
    m_rotate_mouse_end = m_rotate_mouse_start;
    break;
  }
}

void Trackball::key_down(int key) {
  if (!m_enabled) {
    return;
  }

  if (m_mode != MODE::NONE) {
    return;
  }

  if (key == Qt::Key_A && m_rotation_enabled) {
    m_mode = MODE::ROTATE;
    spdlog::debug("Entered rotate");
    return;
  }

  if (key == Qt::Key_S && m_zoom_enabled) {
    m_mode = MODE::ZOOM;
    spdlog::debug("Entered zoom");
    return;
  }

  if (key == Qt::Key_D && m_pan_enabled) {
    m_mode = MODE::PAN;
    spdlog::debug("Entered pan");
    return;
  }
}

void Trackball::mouse_wheel(float xoffset, float yoffset) {
  if (!m_enabled) {
    return;
  }

  if (!m_zoom_enabled) {
    return;
  }
  auto delta_y = 0.0f;
  if (yoffset != 0.0) {
    delta_y = yoffset / 3.0f;
  }
  m_zoom_mouse_start.setY(m_zoom_mouse_start.y() + (delta_y * 0.05f));
}

void Trackball::mouse_move(int xpos, int ypos) {
  if (!m_enabled) {
    return;
  }

  switch (m_mode) {
  case MODE::NONE:return;

  case MODE::ROTATE:m_rotate_mouse_end = get_mouse_projection_on_ball(xpos, ypos);
    break;

  case MODE::ZOOM:m_zoom_mouse_end = normalize_mouse_coords(xpos, ypos);
    break;

  case MODE::PAN:m_pan_mouse_end = normalize_mouse_coords(xpos, ypos);
    break;
  }
  m_cam_view_matrix_is_dirty = true;
}
