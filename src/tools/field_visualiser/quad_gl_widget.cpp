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

  float old_line_width;
  glGetFloatv(GL_LINE_WIDTH, &old_line_width);

  glLineWidth(3.0f);
  glColor4d(1.0, 0.0, 0.0, 1.0);
  glBegin(GL_LINES);
  for (const auto &edge : m_red_edges) {
    unsigned int from_index = edge.first.second;
    unsigned int to_index = edge.second.second;
    glVertex3f(m_vertices.at(from_index * 3 + 0),
               m_vertices.at(from_index * 3 + 1),
               m_vertices.at(from_index * 3 + 2));
    glVertex3f(m_vertices.at(to_index * 3 + 0),
               m_vertices.at(to_index * 3 + 1),
               m_vertices.at(to_index * 3 + 2));
  }
  glEnd();

  checkGLError("maybe_draw_red_edges");
  glLineWidth(old_line_width);
}

void
quad_gl_widget::maybe_draw_selected_edge() const {
  if (m_selected_edge < 0) {
    return;
  }
  if (m_selected_edge_is_red && !m_show_red_edges) {
    return;
  }
  if (!m_selected_edge_is_red && !m_show_blue_edges) {
    return;
  }

  float old_line_width;
  glGetFloatv(GL_LINE_WIDTH, &old_line_width);

  glLineWidth(5.0f);
  glColor4d(1.0, 1.0, 0.0, 1.0);
  glBegin(GL_LINES);
  auto &edge = m_all_edges[m_selected_edge];
  auto &from_vertex = edge.first;
  auto &to_vertex = edge.second;
  glVertex3f(from_vertex[0], from_vertex[1], from_vertex[2]);
  glVertex3f(to_vertex[0], to_vertex[1], to_vertex[2]);
  glEnd();

  checkGLError("maybe_draw_selected_edges");
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

    checkGLError("maybe_draw_blue_edges");
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
  maybe_draw_selected_edge();
}

void
quad_gl_widget::setData(const std::vector<float> &vertices,
                        const std::vector<std::pair<std::pair<std::string, unsigned int>,
                                                    std::pair<std::string, unsigned int>>> &red_edges,
                        const std::vector<std::pair<std::pair<std::string, unsigned int>,
                                                    std::pair<std::string, unsigned int>>> &blue_edges
) {
  using namespace std;

  m_vertices.clear();
  m_red_edges.clear();
  m_blue_edges.clear();
  m_all_edges.clear();

  m_vertices.insert(m_vertices.begin(), vertices.begin(), vertices.end());
  m_red_edges.insert(m_red_edges.begin(), red_edges.begin(), red_edges.end());
  m_blue_edges.insert(m_blue_edges.begin(), blue_edges.begin(), blue_edges.end());
  for (const auto &edge : red_edges) {

    m_all_edges.emplace_back(make_pair<Eigen::Vector3f, Eigen::Vector3f>(
        {vertices[edge.first.second * 3 + 0],
         vertices[edge.first.second * 3 + 1],
         vertices[edge.first.second * 3 + 2]},
        {vertices[edge.second.second * 3 + 0],
         vertices[edge.second.second * 3 + 1],
         vertices[edge.second.second * 3 + 2]}
    ));
  }
  for (const auto &edge: blue_edges) {
    m_all_edges.emplace_back(make_pair<Eigen::Vector3f, Eigen::Vector3f>(
        {vertices[edge.first.second * 3 + 0],
         vertices[edge.first.second * 3 + 1],
         vertices[edge.first.second * 3 + 2]},
        {vertices[edge.second.second * 3 + 0],
         vertices[edge.second.second * 3 + 1],
         vertices[edge.second.second * 3 + 2]}
    ));
  }
  update();
}

void
quad_gl_widget::mouse_moved(unsigned int pixel_x, unsigned int pixel_y) {
}

void
quad_gl_widget::mouseMoveEvent(QMouseEvent *event) {
  float distance = 0;
  m_selected_edge = find_closest_edge(event->x(), event->y(), m_all_edges, distance);
  if (m_selected_edge == -1) {
    emit no_edge_selected();
    return;
  }

  if (m_selected_edge < m_red_edges.size() && !m_show_red_edges) {
    emit no_edge_selected();
    return;
  }

  if (m_selected_edge >= m_red_edges.size() && !m_show_blue_edges) {
    emit no_edge_selected();
    return;
  }

  m_selected_edge_is_red = m_selected_edge < m_red_edges.size();
  spdlog::info("Nearest edge {} ({}): ({}, {}, {}) to ({} {} {})",
               m_selected_edge,
               m_selected_edge_is_red ? "red" : "blue",
               m_all_edges[m_selected_edge].first[0],
               m_all_edges[m_selected_edge].first[1],
               m_all_edges[m_selected_edge].first[2],
               m_all_edges[m_selected_edge].second[0],
               m_all_edges[m_selected_edge].second[1],
               m_all_edges[m_selected_edge].second[2]);

  const auto &edge = m_selected_edge_is_red
                     ? m_red_edges[m_selected_edge]
                     : m_blue_edges[m_selected_edge - m_red_edges.size()];

  const auto &first_surfel_idx = edge.first.second;
  const auto &first_surfel_name = edge.first.first;
  const auto &second_surfel_idx = edge.second.second;
  const auto &second_surfel_name = edge.second.first;
  emit edge_selected(first_surfel_name, second_surfel_name);
}