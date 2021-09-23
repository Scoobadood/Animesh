/**
* Initial @author Eberhard Graether / http://egraether.com/
* C++ port by Michael Ivanov (sasmaster)
*/

#pragma once

#include <cstdint>
#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QObject>
#include <QEvent>
#include <Qt>
#include "AbstractArcBall.h"
#include <spdlog/spdlog.h>

class Trackball : public AbstractArcBall {

public:
  explicit Trackball(//
      const QVector3D &cam_pos = {10, 0, 0}, //
      int m_screen_x = 0, //
      int m_screen_y = 0, //
      int m_screen_width = 100, //
      int m_screen_height = 100 //
  );

  // --- Event Handling ---
  bool eventFilter(QObject *o, QEvent *e) override;
  void mouse_up();
  void mouse_down(Qt::MouseButton button, Qt::KeyboardModifiers mods, int xpos, int ypos);
  void mouse_move(int xpos, int ypos);
  void mouse_wheel(float xoffset, float yoffset);
  void key_down(int key);
  void key_up();

  // -- Matrix transform
  void maybe_update_matrix();
  void get_model_view_matrix(float mat[16]) override;
  QVector3D get_camera_origin() const override;

private:
  QVector3D get_mouse_projection_on_ball(int mouse_x, int mouse_y);
  QVector2D normalize_mouse_coords(int mouse_x, int mouse_y) const;
  void maybe_update_rotation();
  void maybe_update_zoom();
  void maybe_update_pan();

  void CheckDistances();

  enum class MODE : uint8_t {
    NONE,
    ROTATE,
    ZOOM,
    PAN
  };
  MODE m_mode;

  int m_screen_origin_x;
  int m_screen_origin_y;
  int m_screen_width;
  int m_screen_height;

  QVector3D m_cam_location;
  QVector3D m_cam_up;
  QVector3D m_cam_target;
  QMatrix4x4 m_cam_view_matrix;
  bool m_cam_view_matrix_is_dirty;


  QVector3D m_focal_point;
  QVector3D m_lastPos;
  QVector3D m_target_to_cam;

  // Zooming
  QVector2D m_zoom_mouse_start;
  QVector2D m_zoom_mouse_end;
  float m_zoom_speed;
  bool m_zoom_enabled;

  // Rotation
  QVector3D m_rotate_mouse_start;
  QVector3D m_rotate_mouse_end;
  float m_rotate_speed;
  bool m_rotation_enabled;

  // Panning
  QVector2D m_pan_mouse_start;
  QVector2D m_pan_mouse_end;
  float m_pan_speed;
  bool m_pan_enabled;

  float m_damping_factor;
  float m_minDistance;
  float m_maxDistance;
  bool m_enabled;
  bool m_no_roll;
  bool m_static_moving;
};

/**
 * Convert the mouse coords to [0,1]
 */
inline QVector2D
Trackball::normalize_mouse_coords(int mouse_x, int mouse_y) const {
  return {
      static_cast<float>(mouse_x - m_screen_origin_x) / static_cast<float>(m_screen_width),
      static_cast<float>(mouse_y - m_screen_origin_y) / static_cast<float>(m_screen_height)
  };
}

inline void Trackball::mouse_up() {
  if (!m_enabled)
    return;

  m_mode = MODE::NONE;
}

inline void Trackball::key_up() {
  if (!m_enabled) {
    return;
  }
  m_mode = MODE::NONE;
}