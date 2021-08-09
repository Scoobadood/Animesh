//
// Created by Dave Durbin (Old) on 28/6/21.
//

#include <spdlog/spdlog.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Geom/Geom.h>
#include "field_gl_widget.h"

const float DEG2RAD = (3.14159265f / 180.0f);

field_gl_widget::field_gl_widget(
    QWidget *parent, Qt::WindowFlags f) :
    QOpenGLWidget{parent, f} //
    , m_fov{35} //
    , m_zNear{.1f} //
    , m_zFar{500.0f} //
    , m_aspectRatio{1.0f} //
    , m_projectionMatrixIsDirty{true} //
    , m_render_mouse_ray{true} //
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
  float xx[16];
  m_arcBall->get_model_view_matrix(xx);

  bool dirty = false;
  for( int i=0; i<16; i++ ) {
    if( m_model_view_matrix[i] != xx[i]) {
      dirty = true;
      break;
    }
  }
  if( dirty ) {
    for( int i=0; i<16; i++ ) {
      m_model_view_matrix[i] = xx[i];
    }
    spdlog::info(
        "\n{:4.2f} {:4.2f} {:4.2f} {:4.2f}"
        "\n{:4.2f} {:4.2f} {:4.2f} {:4.2f}"
        "\n{:4.2f} {:4.2f} {:4.2f} {:4.2f}"
        "\n{:4.2f} {:4.2f} {:4.2f} {:4.2f}\n",
        m_model_view_matrix[0], m_model_view_matrix[1], m_model_view_matrix[2], m_model_view_matrix[3],
        m_model_view_matrix[4], m_model_view_matrix[5], m_model_view_matrix[6], m_model_view_matrix[7],
        m_model_view_matrix[8], m_model_view_matrix[9], m_model_view_matrix[10], m_model_view_matrix[11],
        m_model_view_matrix[12], m_model_view_matrix[13], m_model_view_matrix[14], m_model_view_matrix[15]
    );
    glLoadMatrixf(m_model_view_matrix);
  }
  checkGLError("update_model_matrix");
}

/**
 * If the projection matrix is dirty, update and reload it.
 */
void
field_gl_widget::maybe_update_projection_matrix() {
  if (m_projectionMatrixIsDirty) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projection_matrix);
    checkGLError("maybe_update_projection_matrix");
    m_projectionMatrixIsDirty = false;
  }
}

void field_gl_widget::maybe_render_mouse_ray() {
  if (!m_render_mouse_ray) {
    return;
  }
  glColor4d(1.0, 0.0, 1.0, 1.0);

  float old_line_width;
  glGetFloatv(GL_LINE_WIDTH, &old_line_width);
  glLineWidth(15.0f);

  float old_point_size;
  glGetFloatv(GL_POINT_SIZE, &old_point_size);
  glPointSize(20.0f);

  glBegin(GL_POINTS);
  glVertex3f(m_near_point[0], m_near_point[1], m_near_point[2]);
  glVertex3f(m_far_point[0], m_far_point[1], m_far_point[2]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3f(m_near_point[0], m_near_point[1], m_near_point[2]);
  glVertex3f(m_far_point[0], m_far_point[1], m_far_point[2]);
  glEnd();

  checkGLError("maybe_render_mouse_ray");
  glLineWidth(old_line_width);
  glPointSize(old_point_size);
}

void
field_gl_widget::paintGL() {
  clear();

  maybe_update_projection_matrix();

  update_model_matrix();

  maybe_render_mouse_ray();

  do_paint();
}

/**
 * Resize the viewport when the window resizes.
 */
void
field_gl_widget::resizeGL(int width, int height) {
  glViewport(0, 0, width, height);
  spdlog::info("ResizeGL parms: ({}, {}) window: ({}, {})",
               width, height, this->width(), this->height()
  );

  m_aspectRatio = (float) width / (float) height;
  auto fovy_rad = m_fov * DEG2RAD;
  const auto yMax = m_zNear * std::tanf(fovy_rad * 2.0f);
  const auto yMin = -yMax;
  const auto xMin = -yMax * m_aspectRatio;
  const auto xMax = yMax * m_aspectRatio;
  // Frustum Equiv.
  const auto temp = 2.0f * m_zNear;
  const auto temp2 = xMax - xMin;
  const auto temp3 = yMax - yMin;
  const auto temp4 = m_zFar - m_zNear;

  m_projection_matrix[0] = temp / temp2;
  m_projection_matrix[1] = 0.0;
  m_projection_matrix[2] = 0.0;
  m_projection_matrix[3] = 0.0;
  m_projection_matrix[4] = 0.0;
  m_projection_matrix[5] = temp / temp3;
  m_projection_matrix[6] = 0.0;
  m_projection_matrix[7] = 0.0;
  m_projection_matrix[8] = (xMax + xMin) / temp2;
  m_projection_matrix[9] = (yMax + yMin) / temp3;
  m_projection_matrix[10] = (-m_zFar - m_zNear) / temp4;
  m_projection_matrix[11] = -1.0;
  m_projection_matrix[12] = 0.0;
  m_projection_matrix[13] = 0.0;
  m_projection_matrix[14] = (-temp * m_zFar) / temp4;
  m_projection_matrix[15] = 0.0;
  m_projectionMatrixIsDirty = true;
}

void
field_gl_widget::checkGLError(const std::string &context) {
  auto err = glGetError();
  if (!err)
    return;
  spdlog::error("{}: {} ", context, err);
}

bool
glUnprojectf(float winx, float winy, float winz,
             const QMatrix4x4& modelview,
             const QMatrix4x4& projection,
             const int *viewport,
             float *objectCoordinate) {

  // Transformation of normalized coordinates between -1 and 1
  QVector4D in{
      (winx - (float) viewport[0]) / (float) viewport[2] * 2.0f - 1.0f,
      (winy - (float) viewport[1]) / (float) viewport[3] * 2.0f - 1.0f,
      2.0f * winz - 1.0f,
      1.0f};

  auto unprojected = in * projection.inverted();
  auto out = unprojected * modelview.inverted();

  if (out[3] == 0.0)
    return false;

  auto scale = 1.0f / out[3];
  objectCoordinate[0] = out[0] * scale;
  objectCoordinate[1] = out[1] * scale;
  objectCoordinate[2] = out[2] * scale;
  return true;
}

Eigen::Vector3f
field_gl_widget::ray_for_pixel(int pixel_x, int pixel_y) {

  makeCurrent();
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);// viewport will hold x,y,w,h
  spdlog::info("ray_for_pixel viewport : {},{}, {},{}", viewport[0], viewport[1], viewport[2], viewport[3]);

  viewport[2] = width();
  viewport[3] = height();
  GLfloat winX, winY;               // Holds Our X, Y and Z Coordinates
  winX = (float) pixel_x;                  // Holds The Mouse X Coordinate
  winY = (float) pixel_y;                  // Holds The Mouse Y Coordinate
  winY = (float) viewport[3] - winY;

  float xyz_far[3];
  float xyz_near[3];
  QMatrix4x4 mv{m_model_view_matrix};
  QMatrix4x4 p{m_projection_matrix};
  glUnprojectf(winX, winY, 1.0f, mv, p, viewport, xyz_far);
  glUnprojectf(winX, winY, 0.0f, mv, p, viewport, xyz_near);

  m_near_point = {xyz_near[0], xyz_near[1], xyz_near[2]};
  m_far_point = {xyz_far[0], xyz_far[1], xyz_far[2]};
  return m_far_point - m_near_point;
}

int
field_gl_widget::find_closest_vertex(unsigned int pixel_x, unsigned int pixel_y,
                                     std::vector<float> &items, float &distance) {
  spdlog::info("pixel {},{}", pixel_x, pixel_y);

  float x = ((2.0f * (float) pixel_x) / (float) width()) - 1.0f;
  float y = 1.0f - ((2.0f * (float) pixel_y) / (float) height());

  Eigen::Vector4f ray_clip{x, y, -1.0f, 1.0f};
  // Recover the projection matrix
  Eigen::Matrix4f projection{m_projection_matrix};
  Eigen::Vector4f ray_eye = projection.inverse() * ray_clip;
  ray_eye[2] = -1.f;
  ray_eye[3] = 0.0f;

  Eigen::Matrix4f model_matrix{m_model_view_matrix};
  auto ray_wor4 = model_matrix.inverse() * ray_eye;
  Eigen::Vector3f ray_wor{ray_wor4[0], ray_wor4[1], ray_wor4[2]};

  ray_wor.normalize();
  spdlog::info("  ray_wor {},{},{}", ray_wor[0], ray_wor[1], ray_wor[2]);

  auto ray_un = ray_for_pixel(pixel_x, pixel_y).normalized();
  spdlog::info("  ray_un {},{},{}", ray_un[0], ray_un[1], ray_un[2]);

  auto co = m_arcBall->get_camera_origin();
  Eigen::Vector3f camera_origin{co[0], co[1], co[2]};

  distance = std::numeric_limits<float>::max();
  int closest_idx = -1;
  for (int idx = 0; idx < items.size() / 3; ++idx) {
    auto dist = distance_from_point_to_line(
        {items[idx * 3], items[idx * 3 + 1], items[idx * 3 + 2]},
        camera_origin, ray_un);
    if (dist < distance) {
      distance = dist;
      closest_idx = idx;
    }
  }
  return closest_idx;
}