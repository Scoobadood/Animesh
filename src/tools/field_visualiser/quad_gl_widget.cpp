//
// Created by Dave Durbin (Old) on 28/6/21.
//

#include <spdlog/spdlog.h>
#include "quad_gl_widget.h"

const float DEG2RAD = (3.14159265f / 180.0f);

quad_gl_widget::quad_gl_widget(
    QWidget *parent, Qt::WindowFlags f) //
    : field_gl_widget(parent, f) //
    , m_show_blue_edges{true} //
    , m_show_red_edges{true} //
{
  using namespace std;

  setFocus();
  setMouseTracking(true);
  setData(
      vector<float>{0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0, 0.0, 0.0},
      vector<pair<pair<string, unsigned int>, pair<string, unsigned int>>>{{{"v1", 0}, {"v2", 1}}},
      vector<pair<pair<string, unsigned int>, pair<string, unsigned int>>>{{{"v1", 0}, {"v3", 2}}}
  );
}

void
quad_gl_widget::drawVertices() const {
  glEnable(GL_POINT_SMOOTH);
  float oldPointSize;
  glGetFloatv(GL_POINT_SIZE, &oldPointSize);

  if (m_selected_vertex >= 0) {
    glPointSize(10.0f);
    glColor4d(1., 1., 0.0, 1.0);
    glBegin(GL_POINTS);
    glVertex3f(m_vertices.at(m_selected_vertex * 3 + 0),
               m_vertices.at(m_selected_vertex * 3 + 1),
               m_vertices.at(m_selected_vertex * 3 + 2));
    glEnd();
  }
  glColor4d(0.8, 0.8, 0.8, 1.0);
  glPointSize(5.0f);
  for (unsigned int i = 0; i < m_vertices.size() / 3; ++i) {
    glBegin(GL_POINTS);
    glVertex3f(m_vertices.at(i * 3 + 0),
               m_vertices.at(i * 3 + 1),
               m_vertices.at(i * 3 + 2));
    glEnd();
  }
  glPointSize(oldPointSize);
  glDisable(GL_POINT_SMOOTH);
  checkGLError("drawPositions");
}

void
quad_gl_widget::maybe_draw_red_edges() const {
  if (!m_show_red_edges) {
    return;
  }
  glColor4d(1.0, 0.0, 0.0, 1.0);

  float old_line_width;
  glGetFloatv(GL_LINE_WIDTH, &old_line_width);
  glLineWidth(3.0f);

  for (const auto &edge : m_red_edges) {
    unsigned int from_index = edge.first.second;
    unsigned int to_index = edge.second.second;
    if (m_selected_vertex > 0) {
      if (m_selected_vertex == from_index || m_selected_vertex == to_index) {
        if (m_selected_vertex == from_index) {
          emit other_vertex(to_index);
        } else {
          emit other_vertex(from_index);
        }
        glLineWidth(5.0f);
        glColor4d(1.0, 1.0, 0.0, 1.0);
        glBegin(GL_LINES);
        glVertex3f(m_vertices.at(from_index * 3 + 0),
                   m_vertices.at(from_index * 3 + 1),
                   m_vertices.at(from_index * 3 + 2));
        glVertex3f(m_vertices.at(to_index * 3 + 0),
                   m_vertices.at(to_index * 3 + 1),
                   m_vertices.at(to_index * 3 + 2));
        glEnd();
        glLineWidth(3.0f);
        glColor4d(1.0, 0.0, 0.0, 1.0);
      }
    }
    glBegin(GL_LINES);
    glVertex3f(m_vertices.at(from_index * 3 + 0),
               m_vertices.at(from_index * 3 + 1),
               m_vertices.at(from_index * 3 + 2));
    glVertex3f(m_vertices.at(to_index * 3 + 0),
               m_vertices.at(to_index * 3 + 1),
               m_vertices.at(to_index * 3 + 2));
    glEnd();

    checkGLError("maybe_draw_red_edges");
  }
  glLineWidth(old_line_width);
}

void
quad_gl_widget::maybe_draw_blue_edges() const {
  if (!m_show_blue_edges) {
    return;
  }
  glColor4d(0.0, 0.0, 1.0, 1.0);

  float old_line_width;
  glGetFloatv(GL_LINE_WIDTH, &old_line_width);
  glLineWidth(3.0f);

  for (const auto &edge : m_blue_edges) {
    unsigned int from_index = edge.first.second;
    unsigned int to_index = edge.second.second;
    glBegin(GL_LINES);
    glVertex3f(m_vertices.at(from_index * 3 + 0),
               m_vertices.at(from_index * 3 + 1),
               m_vertices.at(from_index * 3 + 2));
    glVertex3f(m_vertices.at(to_index * 3 + 0),
               m_vertices.at(to_index * 3 + 1),
               m_vertices.at(to_index * 3 + 2));
    glEnd();

    checkGLError("maybe_draw_red_edges");
  }
  glLineWidth(old_line_width);
}

void
quad_gl_widget::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  checkGLError("Generating texture");
}

void
quad_gl_widget::do_paint() {
  drawVertices();

  maybe_draw_red_edges();
  maybe_draw_blue_edges();
}

void
quad_gl_widget::setData(const std::vector<float> &vertices,
                        const std::vector<std::pair<std::pair<std::string, unsigned int>,
                                                    std::pair<std::string, unsigned int>>> &red_edges,
                        const std::vector<std::pair<std::pair<std::string, unsigned int>,
                                                    std::pair<std::string, unsigned int>>> &blue_edges
) {
  m_vertices.clear();
  m_red_edges.clear();
  m_blue_edges.clear();

  m_vertices.insert(m_vertices.begin(), vertices.begin(), vertices.end());
  m_red_edges.insert(m_red_edges.begin(), red_edges.begin(), red_edges.end());
  m_blue_edges.insert(m_blue_edges.begin(), blue_edges.begin(), blue_edges.end());
  update();
}

void
quad_gl_widget::mouse_moved(unsigned int pixel_x, unsigned int pixel_y) {
}

void
quad_gl_widget::mouseMoveEvent(QMouseEvent *event) {
  float distance = 0;
  m_selected_vertex = find_closest_vertex(event->x(), event->y(), m_vertices, distance);
  spdlog::info("Nearest vertex : {}, {}, {}",
               m_vertices[m_selected_vertex * 3],
               m_vertices[m_selected_vertex * 3 + 1],
               m_vertices[m_selected_vertex * 3 + 2]);
  emit vertex_selected(m_selected_vertex);
}