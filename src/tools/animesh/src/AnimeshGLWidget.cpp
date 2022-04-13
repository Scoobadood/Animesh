#include "AnimeshGLWidget.h"
#include "AnimeshApp.h"
#include <vector>
#include <Geom/Geom.h>
#include <spdlog/spdlog.h>
#include <QCoreApplication>
#include <AnimeshWindow.h>
#include <ArcBall/AbstractArcBall.h>
#include <Quad/Quad.h>
#include <Eigen/Geometry>

const float DEG2RAD = (3.14159265358979323846264f / 180.0f);
const float FIELD_OFFSET_FACTOR = 0.1f;

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
    , m_vertex_colour(255, 255, 255, 255) //
    , m_normal_colour(255, 0, 0, 255) //
    , m_tangent_colour(127, 127, 127, 255) //
    , m_main_tangent_colour(255, 255, 255, 255) //
    , m_posy_vertex_colour(0, 255, 127, 255) //
    , m_posy_surface_colour(0, 150, 80, 255) //
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
  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glLineWidth(1.0f);
  checkGLError("initialiseGL");
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
AnimeshGLWidget::set_drawing_colour(const QColor &colour) {
  glColor4f(static_cast<float>(colour.redF()),
            static_cast<float>(colour.greenF()),
            static_cast<float>(colour.blueF()),
            1.0f);
}

void
AnimeshGLWidget::maybe_draw_vertex_positions() const {
  if (!m_show_vertices) {
    return;
  }

  AnimeshWindow *window = nullptr;
  const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
  for (QWidget *widget: topLevelWidgets) {
    if ("AnimeshWindow" == widget->objectName()) {
      window = qobject_cast<AnimeshWindow *>(widget);
      break;
    }
  }

  if (window == nullptr) {
    return;
  }

  auto graph = window->graph();
  if (graph == nullptr) {
    return;
  }

  float old_point_size;
  glGetFloatv(GL_POINT_SIZE, &old_point_size);

  glPointSize(3.0f);

  Eigen::Vector3f v, t, n;
  set_drawing_colour(m_vertex_colour);
  glBegin(GL_POINTS);
  for (const auto &node: graph->nodes()) {
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    v = v + (n * m_scale * FIELD_OFFSET_FACTOR);
    glVertex3f(v.x(), v.y(), v.z());
  }
  glEnd();
  glPointSize(old_point_size);
  checkGLError("maybe_draw_vertex_positions");
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
    v = v + (n * m_scale * FIELD_OFFSET_FACTOR);
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
    v = v + (n * m_scale * FIELD_OFFSET_FACTOR);
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

void AnimeshGLWidget::maybe_draw_consensus_graph() const {
  if (!m_show_consensus_graph) {
    return;
  }

  auto window = qobject_cast<AnimeshWindow *>(QApplication::topLevelWidgets()[0]);
  if (window == nullptr) {
    return;
  }

  auto consensus_graph = window->consensus_graph();
  if (consensus_graph == nullptr) {
    return;
  }

  // TODO: Draw it
  glBegin(GL_LINES);

  for (const auto &edge: consensus_graph->edges()) {
    if (*edge.data() == EDGE_TYPE_RED) {
      set_drawing_colour(QColorConstants::Red);
    } else if (*edge.data() == EDGE_TYPE_BLU) {
      set_drawing_colour(QColorConstants::Blue);
    }

    const auto &v1 = edge.from()->data().location;
    const auto &v2 = edge.to()->data().location;
    glVertex3f(v1.x(), v1.y(), v1.z());
    glVertex3f(v2.x(), v2.y(), v2.z());
  }
  glEnd();
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
    v = v + (n * m_scale * FIELD_OFFSET_FACTOR);
    t = t * m_scale;
    glVertex3f(v.x(), v.y(), v.z());
    glVertex3f(v.x() + t.x(), v.y() + t.y(), v.z() + t.z());
  }
  glEnd();
  checkGLError("maybe_draw_main_tangents");
}

void
AnimeshGLWidget::maybe_draw_posy_vertices() const {
  if (!m_show_posy_vertices) {
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

  float old_point_size;
  glGetFloatv(GL_POINT_SIZE, &old_point_size);
  glPointSize(3.0f);

  Eigen::Vector3f v, t, n;
  glBegin(GL_POINTS);
  set_drawing_colour(m_posy_vertex_colour);
  for (const auto &node: graph->nodes()) {
    node->data()->get_vertex_tangent_normal_for_frame(0, v, t, n);
    auto offset = node->data()->reference_lattice_offset();
    auto posy_vertex = v + (t * offset[0]) + (n.cross(t) * offset[1]);
    glVertex3f(posy_vertex.x(), posy_vertex.y(), posy_vertex.z());
  }
  glEnd();

  glPointSize(old_point_size);
  checkGLError("maybe_draw_posy_vertices");
}

void
AnimeshGLWidget::maybe_draw_surface() {
  if (!m_show_surface) {
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

  auto surface = window->surface();
  if (surface.empty()) {
    return;
  }

  glBegin(GL_TRIANGLES);
  set_drawing_colour(m_posy_surface_colour);
  for (int f = 0; f < surface.size(); f += 3) {
    glVertex3f(surface[f], surface[f + 1], surface[f + 2]);
  }
  glEnd();

  checkGLError("maybe_draw_surface");

}

/*
 * Taken from https://stackoverflow.com/questions/47949485/sorting-a-list-of-3d-points-in-clockwise-order
 * let n be the normal vector
r := vertices[0] - c // use an arbitrary vector as the twelve o’clock reference
p := cross(r, n)     // get the half-plane partition vector

// returns true if v1 is clockwise from v2 around c
function less(v1, v2):
    u1 := v1 - c
    u2 := v2 - c
    h1 := dot(u1, p)
    h2 := dot(u2, p)

    if      h2 ≤ 0 and h1 > 0:
        return false

    else if h1 ≤ 0 and h2 > 0:
        return true

    else if h1 = 0 and h2 = 0:
        return dot(u1, r) > 0 and dot(u2, r) < 0

    else:
        return dot(cross(u1, u2), c) > 0

    //           h2 > 0     h2 = 0     h2 < 0
    //          ————————   ————————   ————————
    //  h1 > 0 |   *        v1 > v2    v1 > v2
    //  h1 = 0 | v1 < v2       †          *
    //  h1 < 0 | v1 < v2       *          *

    //  *   means we can use the triple product because the (cylindrical)
    //      angle between u1 and u2 is less than π
    //  †   means u1 and u2 are either 0 or π around from the zero reference
    //      in which case u1 < u2 only if dot(u1, r) > 0 and dot(u2, r) < 0
 */
void
order_3d_points(const Eigen::Vector3f &normal, const Eigen::Vector3f &centre, std::vector<Eigen::Vector3f> points) {
  const auto r = points[0] - centre; // use an arbitrary vector as the twelve o’clock reference
  const auto p = r.cross(normal);     // get the half-plane partition vector
  std::sort(points.begin(), points.end(), [&](const Eigen::Vector3f &v1, const Eigen::Vector3f &v2) {
    const auto u1 = v1 - centre;
    const auto u2 = v2 - centre;
    const auto h1 = u1.dot(p);
    const auto h2 = u2.dot(p);
    if (h2 <= 0 && h1 > 0) return false;
    if (h1 <= 0 && h2 > 0) return true;
    if (h1 == 0 && h2 == 0) {
      return u1.dot(r) > 0 && u2.dot(r) < 0;
    }
    return u1.cross(u2).dot(centre) > 0;
  });
}

void
AnimeshGLWidget::paintGL() {
  clear();

  maybe_update_projection_matrix();

  update_model_matrix();

  // Actual painting follows here
  maybe_draw_surface();
  maybe_draw_vertex_positions();
  maybe_draw_normals();
  maybe_draw_tangents();
  maybe_draw_main_tangents();
  maybe_draw_posy_vertices();
  maybe_draw_consensus_graph();
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

