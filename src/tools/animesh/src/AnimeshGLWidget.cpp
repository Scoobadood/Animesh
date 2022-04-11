#include "AnimeshGLWidget.h"
#include "AnimeshApp.h"
#include <vector>
#include <Geom/Geom.h>
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include <AnimeshWindow.h>
#include <ArcBall/AbstractArcBall.h>

const float DEG2RAD = (3.14159265358979323846264f / 180.0f);

AnimeshGLWidget::AnimeshGLWidget(
    QWidget *parent, //
    Qt::WindowFlags f) //
    : QOpenGLWidget{parent, f} //
    , m_fov{15} //
    , m_z_near{.5f} //
    , m_z_far{1000.0f} //
    , m_aspect_ratio{1.0f} //
    , m_projection_matrix_is_dirty{true} //
    , m_projection_matrix{} //
    , m_model_view_matrix{} //
    , m_normal_colour(255, 0, 0) //
    , m_tangent_colour(255, 255, 255, 255) //
    , m_main_tangent_colour(127, 127, 127) //
    , m_show_normals{false} //
    , m_show_tangents{false} //
    , m_show_main_tangents{false} //
    , m_scale{1.0f} //
{
  // Forces capture of keyboard input
  setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  setFocus();
}

void
AnimeshGLWidget::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  glLineWidth(3.0f);
}

void
AnimeshGLWidget::clear() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  checkGLError("clear");
}

/**
 * If the ModelView matrix is dirty, update and reload it.
 */
void
AnimeshGLWidget::update_model_matrix() {
  glMatrixMode(GL_MODELVIEW);
  float xx[16];
  m_arc_ball->get_model_view_matrix(xx);

  bool dirty = false;
  for (int i = 0; i < 16; i++) {
    if (m_model_view_matrix[i] != xx[i]) {
      dirty = true;
      break;
    }
  }
  if (dirty) {
    for (int i = 0; i < 16; i++) {
      m_model_view_matrix[i] = xx[i];
    }
    glLoadMatrixf(m_model_view_matrix);
  }
  checkGLError("update_model_matrix");
}

/**
 * If the projection matrix is dirty, update and reload it.
 */
void
AnimeshGLWidget::maybe_update_projection_matrix() {
  spdlog::trace("maybe_update_projection_matrix()");
  if (m_projection_matrix_is_dirty) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_projection_matrix);
    checkGLError("maybe_update_projection_matrix");
    m_projection_matrix_is_dirty = false;
  }
}

void
AnimeshGLWidget::draw_vertex_positions() {
  auto window = qobject_cast<AnimeshWindow *>(QApplication::topLevelWidgets()[0]);
  if (window == nullptr) {
    return;
  }

  auto graph = window->graph();
  if (graph == nullptr) {
    return;
  }

  glEnable(GL_POINT_SMOOTH);
  float old_point_size;
  glGetFloatv(GL_POINT_SIZE, &old_point_size);

  glPointSize(3.0f);

  Eigen::Vector3f v, t, n;
  glBegin(GL_POINTS);
  for (const auto &node: graph->nodes()) {
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    glVertex3f(v.x(), v.y(), v.z());
  }
  glEnd();
  glPointSize(old_point_size);
  glDisable(GL_POINT_SMOOTH);
  checkGLError("draw_vertex_positions");
}

void
AnimeshGLWidget::maybe_draw_normals() const {
  if (!m_show_normals) {
    return;
  }

  auto window = qobject_cast<AnimeshWindow *>(QApplication::topLevelWidgets()[0]);
  if (window == nullptr) {
    return;
  }

  auto graph = window->graph();
  if (graph == nullptr) {
    return;
  }

  Eigen::Vector3f v, t, n;
  glBegin(GL_LINES);
  set_drawing_colour(m_normal_colour);
  for (const auto &node: graph->nodes()) {
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    n = n * m_scale;
    glVertex3f(v.x(), v.y(), v.z());
    glVertex3f(v.x() + n.x(), v.y() + n.y(), v.z() + n.z());
  }
  glEnd();
  checkGLError("maybe_draw_normals");
}

void
AnimeshGLWidget::maybe_draw_tangents() const {
  if (!m_show_tangents) {
    return;
  }

  auto window = qobject_cast<AnimeshWindow *>(QApplication::topLevelWidgets()[0]);
  if (window == nullptr) {
    return;
  }

  auto graph = window->graph();
  if (graph == nullptr) {
    return;
  }

  Eigen::Vector3f v, t, n;
  glBegin(GL_LINES);
  set_drawing_colour(m_tangent_colour);
  for (const auto &node: graph->nodes()) {
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    t = t * m_scale;
    glVertex3f(v.x(), v.y(), v.z());
    glVertex3f(v.x() + t.x(), v.y() + t.y(), v.z() + t.z());
    glVertex3f(v.x(), v.y(), v.z());
    glVertex3f(v.x() - t.x(), v.y() - t.y(), v.z() - t.z());
    t = n.cross(t);
    glVertex3f(v.x(), v.y(), v.z());
    glVertex3f(v.x() + t.x(), v.y() + t.y(), v.z() + t.z());
    glVertex3f(v.x(), v.y(), v.z());
    glVertex3f(v.x() - t.x(), v.y() - t.y(), v.z() - t.z());
  }
  glEnd();
  checkGLError("maybe_draw_tangents");
}

void
AnimeshGLWidget::set_drawing_colour(const QColor &colour) {
  glColor4f(static_cast<float>(colour.redF()),
            static_cast<float>(colour.greenF()),
            static_cast<float>(colour.blueF()),
            1.0f);
}

void
AnimeshGLWidget::maybe_draw_main_tangents() const {
  if (!m_show_main_tangents) {
    return;
  }

  auto window = qobject_cast<AnimeshWindow *>(QApplication::topLevelWidgets()[0]);
  if (window == nullptr) {
    return;
  }

  auto graph = window->graph();
  if (graph == nullptr) {
    return;
  }

  Eigen::Vector3f v, t, n;
  glBegin(GL_LINES);
  set_drawing_colour(m_main_tangent_colour);
  for (const auto &node: graph->nodes()) {
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    t = t * m_scale;
    glVertex3f(v.x(), v.y(), v.z());
    glVertex3f(v.x() + t.x(),v.y() + t.y(), v.z() + t.z());
  }
  glEnd();
  checkGLError("maybe_draw_main_tangents");
}
void
AnimeshGLWidget::paintGL() {
  clear();

  maybe_update_projection_matrix();

  update_model_matrix();

  // Actual painting follows here
  draw_vertex_positions();
  maybe_draw_normals();
  maybe_draw_tangents();
  maybe_draw_main_tangents();
}

/**
 * Resize the viewport when the window resizes.
 */
void
AnimeshGLWidget::resizeGL(int width, int height) {
  spdlog::trace("resizeGL({}, {})", width, height);

  glViewport(0, 0, width, height);

  m_aspect_ratio = (float) width / (float) height;
  auto fovy_rad = m_fov * DEG2RAD;
  const auto yMax = m_z_near * std::tanf(fovy_rad * 2.0f);
  const auto yMin = -yMax;
  const auto xMin = -yMax * m_aspect_ratio;
  const auto xMax = yMax * m_aspect_ratio;

  // Frustum Equiv.
  const auto temp = 2.0f * m_z_near;
  const auto temp2 = xMax - xMin;
  const auto temp3 = yMax - yMin;
  const auto temp4 = m_z_far - m_z_near;

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
  m_projection_matrix[10] = (-m_z_far - m_z_near) / temp4;
  m_projection_matrix[11] = -1.0;
  m_projection_matrix[12] = 0.0;
  m_projection_matrix[13] = 0.0;
  m_projection_matrix[14] = (-temp * m_z_far) / temp4;
  m_projection_matrix[15] = 0.0;
  m_projection_matrix_is_dirty = true;
}

void
AnimeshGLWidget::checkGLError(const std::string &context) {
  auto err = glGetError();
  if (!err)
    return;
  spdlog::error("{}: {} ", context, err);
}

void
AnimeshGLWidget::set_arc_ball(const std::shared_ptr<AbstractArcBall> &arc_ball) {
  m_arc_ball = arc_ball;
  installEventFilter(m_arc_ball.get());
}

