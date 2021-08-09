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
  m_arcBall->get_model_view_matrix(m_model_view_matrix);
  glLoadMatrixf(m_model_view_matrix);
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
                 Eigen::Matrix4f &modelview,
                 Eigen::Matrix4f &projection,
                 int *viewport,
                 float *objectCoordinate) {

  // Calculation for inverting a matrix, compute projection x modelview
  // and store in A[16]
  const auto A = modelview * projection;
  const auto m = A.inverse();

  // Transformation of normalized coordinates between -1 and 1
  Eigen::Vector4f in{
      (winx - (float) viewport[0]) / (float) viewport[2] * 2.0f - 1.0f,
      (winy - (float) viewport[1]) / (float) viewport[3] * 2.0f - 1.0f,
      2.0f * winz - 1.0f,
      1.0f};

  Eigen::Vector4f out = m * in;
  if (out[3] == 0.0)
    return false;

  out[3] = 1.0f / out[3];
  objectCoordinate[0] = out[0] * out[3];
  objectCoordinate[1] = out[1] * out[3];
  objectCoordinate[2] = out[2] * out[3];
  return true;
}

Eigen::Vector3f
field_gl_widget::ray_for_pixel(int pixel_x, int pixel_y) {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);// viewport will hold x,y,w,h
  spdlog::info("ray_for_pixel viewport : {},{}, {},{}", viewport[0],viewport[1],viewport[2],viewport[3]);
  viewport[2] = width();
  viewport[3] = height();
  Eigen::Matrix4f mv{m_model_view_matrix};
  Eigen::Matrix4f p{m_projection_matrix};

  GLfloat winX, winY;               // Holds Our X, Y and Z Coordinates
  winX = (float) pixel_x;                  // Holds The Mouse X Coordinate
  winY = (float) pixel_y;                  // Holds The Mouse Y Coordinate
  winY = (float) viewport[3] - winY;

  float xyz_far[3];
  float xyz_near[3];
  glUnprojectf(winX, winY, 1.0f, mv, p, viewport, xyz_far);
  glUnprojectf(winX, winY, 0.0f, mv, p, viewport, xyz_near);

  return {xyz_far[0] - xyz_near[0], xyz_far[1] - xyz_near[1], xyz_far[2] - xyz_near[2]};
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
  spdlog::info("  ray_wor {},{},{}", ray_wor[0],  ray_wor[1], ray_wor[2]);

  auto ray_un = ray_for_pixel(pixel_x, pixel_y).normalized();
  spdlog::info("  ray_un {},{},{}", ray_un[0],  ray_un[1], ray_un[2]);

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