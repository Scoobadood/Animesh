#include "rosy_gl_widget.h"
#include "field_visualiser_window.h"
#include <vector>
#include <QColor>
#include <Geom/Geom.h>

rosy_gl_widget::rosy_gl_widget(
    QWidget *parent, //
    Qt::WindowFlags f) //
    : field_gl_widget{parent, f}, m_normalScaleFactor{1.0f} //
    , m_renderNormals{true} //
    , m_renderPath{true} //
    , m_renderSplats{false} //
    , m_renderNeighbours{false} //
    , m_renderMainTangents{true} //
    , m_renderOtherTangents{true} //
    , m_renderErrorColours{true} //
    , m_normalColour{QColor{255, 255, 255, 255}} //
    , m_pathColour{QColor{0, 216, 197, 255}} //
    , m_selected_colour{QColor{200, 10, 200, 255}} //
    , m_splatColour{QColor{88, 116, 167, 255}} //
    , m_neighbourColour{QColor{255, 255, 0, 255}} //
    , m_mainTangentColour{QColor{255, 0, 0, 255}} //
    , m_otherTangentsColour{QColor{0, 255, 0, 255}} //
{
  setFocus();
}

void drawEllipse(float cx, float cy, float cz,
                 float nx, float ny, float nz,
                 float tx, float ty, float tz,
                 float radius,
                 float offset,
                 int num_segments) {

  float angle_delta = (2 * 3.1415926f) / (float) num_segments;
  Eigen::Vector3f axis{nx, ny, nz};
  Eigen::Vector3f point{tx, ty, tz};
  Eigen::Vector3f pos{cx, cy, cz};
  pos -= (offset * axis);

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(pos.x(), pos.y(), pos.z());
  for (int i = 0; i <= num_segments; ++i) {
    auto angle = (float) i * angle_delta;
    auto vec = rotate_point_through_axis_angle(axis, angle, point);
    glVertex3f(pos.x() + vec.x() * radius,
               pos.y() + vec.y() * radius,
               pos.z() + vec.z() * radius);
    glNormal3f(nx, ny, nz);
  }
  glEnd();
}

void
rosy_gl_widget::maybeDrawSplats() const {
  if (!m_renderSplats) {
    return;
  }

  glColor4d(m_splatColour.redF(), m_splatColour.greenF(),
            m_splatColour.blueF(), m_splatColour.alphaF());

  int idx = 0;
  glBegin(GL_TRIANGLES);
  for (auto ts : m_triangle_fan_sizes) {
    for (int i = 0; i < ts; ++i) {
      glVertex3f(m_triangle_fans[idx],
                 m_triangle_fans[idx + 1],
                 m_triangle_fans[idx + 2]);
      glVertex3f(m_triangle_fans[idx + 3],
                 m_triangle_fans[idx + 4],
                 m_triangle_fans[idx + 5]);
      glVertex3f(m_triangle_fans[idx + 6],
                 m_triangle_fans[idx + 7],
                 m_triangle_fans[idx + 8]);
      idx += 9;
    }
  }
  glEnd();

  checkGLError("maybeDrawSplats");
}

void
rosy_gl_widget::maybeDrawPath() const {
  if (!m_renderPath) {
    return;
  }
  if (m_path.empty()) {
    return;
  }

  for (unsigned int i = 0; i < m_path.size() / 13; ++i) {
    glColor4f(m_path.at(i * 13 + 0),
              m_path.at(i * 13 + 1),
              m_path.at(i * 13 + 2),
              m_path.at(i * 13 + 3));
    drawEllipse(m_path.at(i * 13 + 4),
                m_path.at(i * 13 + 5),
                m_path.at(i * 13 + 6),
                m_path.at(i * 13 + 10),
                m_path.at(i * 13 + 11),
                m_path.at(i * 13 + 12),
                m_path.at(i * 13 + 7),
                m_path.at(i * 13 + 8),
                m_path.at(i * 13 + 9),
                0.25, 0.03f, 10
    );
  }
  checkGLError("maybeDrawPath");
}

void
rosy_gl_widget::maybeDrawNormals() const {
  if (!m_renderNormals) {
    return;
  }
  glColor4d(m_normalColour.redF(), m_normalColour.greenF(),
            m_normalColour.blueF(), m_normalColour.alphaF());

  for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
    if (m_renderErrorColours) {
      auto r = m_colours.at(i * 4 + 0);
      auto g = m_colours.at(i * 4 + 1);
      auto b = m_colours.at(i * 4 + 2);
      auto a = m_colours.at(i * 4 + 3);
      glColor4f(r, g, b, a);
    }
    glBegin(GL_LINES);
    glVertex3f(m_positions.at(i * 3 + 0),
               m_positions.at(i * 3 + 1),
               m_positions.at(i * 3 + 2));
    glNormal3f(m_normals.at(i * 3 + 0),
               m_normals.at(i * 3 + 1),
               m_normals.at(i * 3 + 2));
    glVertex3f(m_positions.at(i * 3 + 0) + (m_normals.at(i * 3 + 0) * m_normalScaleFactor),
               m_positions.at(i * 3 + 1) + (m_normals.at(i * 3 + 1) * m_normalScaleFactor),
               m_positions.at(i * 3 + 2) + (m_normals.at(i * 3 + 2) * m_normalScaleFactor));
    glNormal3f(m_normals.at(i * 3 + 0),
               m_normals.at(i * 3 + 1),
               m_normals.at(i * 3 + 2));
    glEnd();
  }
  checkGLError("maybeDrawNormals");

}

void
rosy_gl_widget::drawPositions() const {
  glColor4d(m_normalColour.redF(), m_normalColour.greenF(),
            m_normalColour.blueF(), m_normalColour.alphaF());

  glEnable(GL_POINT_SMOOTH);
  float oldPointSize;
  glGetFloatv(GL_POINT_SIZE, &oldPointSize);

  glPointSize(3.0f);
  for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
    glBegin(GL_POINTS);
    glVertex3f(m_positions.at(i * 3 + 0),
               m_positions.at(i * 3 + 1),
               m_positions.at(i * 3 + 2));
    glEnd();
  }
  glPointSize(oldPointSize);
  glDisable(GL_POINT_SMOOTH);
  checkGLError("drawPositions");
}

void
rosy_gl_widget::maybeHighlightSelectedItem() const {
  if (m_selected_item_data.empty()) {
    return;
  }
  glColor4d(m_selected_colour.redF(),
            m_selected_colour.greenF(),
            m_selected_colour.blueF(),
            m_selected_colour.alphaF());

  float oldPointSize;
  glGetFloatv(GL_POINT_SIZE, &oldPointSize);
  glPointSize(5.0f);
  glEnable(GL_POINT_SMOOTH);
  glBegin(GL_POINTS);
  glVertex3f(m_selected_item_data.at(0),
             m_selected_item_data.at(1),
             m_selected_item_data.at(2));
  glEnd();
  glDisable(GL_POINT_SMOOTH);
  glPointSize(oldPointSize);

  checkGLError("maybeHighlightSelectedItem");
}

void
rosy_gl_widget::maybeDrawNeighbours() const {
  if (!m_renderNeighbours) {
    return;
  }
  if (m_neighbour_data.empty()) {
    return;
  }
  glColor4d(m_neighbourColour.redF(),
            m_neighbourColour.greenF(),
            m_neighbourColour.blueF(),
            m_neighbourColour.alphaF());
  glBegin(GL_LINES);
  for (int i = 0; i < m_neighbour_data.size(); i += 6) {
    glVertex3f(m_neighbour_data.at(i + 0),
               m_neighbour_data.at(i + 1),
               m_neighbour_data.at(i + 2));
    glVertex3f(m_neighbour_data.at(i + 3),
               m_neighbour_data.at(i + 4),
               m_neighbour_data.at(i + 5));
  }
  glEnd();
  checkGLError("maybeDrawNeighbours");
}

void
rosy_gl_widget::maybeDrawMainTangents() const {
  if (!m_renderMainTangents) {
    return;
  }
  glColor4d(m_mainTangentColour.redF(), m_mainTangentColour.greenF(),
            m_mainTangentColour.blueF(), m_mainTangentColour.alphaF());
  for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
    if (m_renderErrorColours) {
      auto r = m_colours.at(i * 4 + 0);
      auto g = m_colours.at(i * 4 + 1);
      auto b = m_colours.at(i * 4 + 2);
      auto a = m_colours.at(i * 4 + 3);
      glColor4f(r, g, b, a);
    }

    glBegin(GL_LINES);
    glVertex3f(m_positions.at(i * 3 + 0),
               m_positions.at(i * 3 + 1),
               m_positions.at(i * 3 + 2));
    glNormal3f(m_normals.at(i * 3 + 0),
               m_normals.at(i * 3 + 1),
               m_normals.at(i * 3 + 2));
    glVertex3f(m_positions.at(i * 3 + 0) + (m_tangents.at(i * 3 + 0) * m_normalScaleFactor),
               m_positions.at(i * 3 + 1) + (m_tangents.at(i * 3 + 1) * m_normalScaleFactor),
               m_positions.at(i * 3 + 2) + (m_tangents.at(i * 3 + 2) * m_normalScaleFactor));
    glNormal3f(m_normals.at(i * 3 + 0),
               m_normals.at(i * 3 + 1),
               m_normals.at(i * 3 + 2));
    glEnd();
  }
  checkGLError("maybeDrawTangents");

}

void
rosy_gl_widget::maybeDrawOtherTangents() const {
  if (!m_renderOtherTangents) {
    return;
  }
  glColor4d(m_otherTangentsColour.redF(),
            m_otherTangentsColour.greenF(),
            m_otherTangentsColour.blueF(),
            m_otherTangentsColour.alphaF());

  for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
    if (m_renderErrorColours) {
      auto r = m_colours.at(i * 4 + 0);
      auto g = m_colours.at(i * 4 + 1);
      auto b = m_colours.at(i * 4 + 2);
      auto a = m_colours.at(i * 4 + 3);
      glColor4d(r, g, b, a);
    }
    // Get perpendicular tangent by computing cross(norm,tan)
    const auto normX = m_normals.at(i * 3 + 0);
    const auto normY = m_normals.at(i * 3 + 1);
    const auto normZ = m_normals.at(i * 3 + 2);
    const auto tanX = m_tangents.at(i * 3 + 0);
    const auto tanY = m_tangents.at(i * 3 + 1);
    const auto tanZ = m_tangents.at(i * 3 + 2);

    auto crossTanX = (normY * tanZ - normZ * tanY) * m_normalScaleFactor; //bn -cm
    auto crossTanY = (normZ * tanX - normX * tanZ) * m_normalScaleFactor; //bn -cm
    auto crossTanZ = (normX * tanY - normY * tanX) * m_normalScaleFactor; //bn -cm

    glBegin(GL_LINES);
    glVertex3f(m_positions.at(i * 3 + 0) - crossTanX,
               m_positions.at(i * 3 + 1) - crossTanY,
               m_positions.at(i * 3 + 2) - crossTanZ);
    glNormal3f(normX, normY, normZ);
    glVertex3f(m_positions.at(i * 3 + 0) + crossTanX,
               m_positions.at(i * 3 + 1) + crossTanY,
               m_positions.at(i * 3 + 2) + crossTanZ);
    glNormal3f(normX, normY, normZ);
    glVertex3f(m_positions.at(i * 3 + 0) - (tanX * m_normalScaleFactor),
               m_positions.at(i * 3 + 1) - (tanY * m_normalScaleFactor),
               m_positions.at(i * 3 + 2) - (tanZ * m_normalScaleFactor));
    glNormal3f(normX, normY, normZ);
    glVertex3f(m_positions.at(i * 3 + 0),
               m_positions.at(i * 3 + 1),
               m_positions.at(i * 3 + 2));
    glNormal3f(normX, normY, normZ);
    glEnd();
  }
  checkGLError("maybeDrawOtherTangents");
}

void
rosy_gl_widget::do_paint() {
  drawPositions();

  maybeDrawNormals();

  maybeDrawMainTangents();

  maybeDrawOtherTangents();

  maybeHighlightSelectedItem();

  maybeDrawNeighbours();

  maybeDrawPath();

  maybeDrawSplats();
}

void
rosy_gl_widget::setRoSyData(const std::vector<float> &positions,
                            const std::vector<float> &normals,
                            const std::vector<float> &tangents,
                            const std::vector<float> &colours,
                            const std::vector<float> &path,
                            const std::vector<float> &triangle_fans,
                            const std::vector<unsigned int> &triangle_fan_sizes,
                            const float normal_scale_factor) {
  m_positions.clear();
  m_tangents.clear();
  m_normals.clear();
  m_colours.clear();
  m_path.clear();
  m_triangle_fans.clear();
  m_triangle_fan_sizes.clear();

  m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
  m_tangents.insert(m_tangents.begin(), tangents.begin(), tangents.end());
  m_normals.insert(m_normals.begin(), normals.begin(), normals.end());
  m_colours.insert(m_colours.begin(), colours.begin(), colours.end());
  m_path.insert(m_path.begin(), path.begin(), path.end());
  m_triangle_fans.insert(m_triangle_fans.begin(), triangle_fans.begin(), triangle_fans.end());
  m_triangle_fan_sizes.insert(m_triangle_fan_sizes.begin(), triangle_fan_sizes.begin(), triangle_fan_sizes.end());
  m_normalScaleFactor = normal_scale_factor;

  const auto &w = window();
  ((field_visualiser_window *) w)->m_rosy_geometry_extractor->update_selection_and_neighbours(m_selected_item_data,
                                                                                              m_neighbour_data);
}

void
rosy_gl_widget::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glEnable(GL_COLOR_MATERIAL);
  glLineWidth(3.0f);
//  enable_light();
}

void
rosy_gl_widget::renderNormals(bool shouldRender) {
  if (m_renderNormals != shouldRender) {
    m_renderNormals = shouldRender;
  }
}

void
rosy_gl_widget::renderPath(bool shouldRender) {
  if (m_renderPath != shouldRender) {
    m_renderPath = shouldRender;
  }
}

void
rosy_gl_widget::renderSplats(bool shouldRender) {
  if (m_renderSplats != shouldRender) {
    m_renderSplats = shouldRender;
  }
}

void
rosy_gl_widget::renderNeighbours(bool shouldRender) {
  if (m_renderNeighbours != shouldRender) {
    m_renderNeighbours = shouldRender;
  }
}

void
rosy_gl_widget::renderMainTangents(bool shouldRender) {
  if (m_renderMainTangents != shouldRender) {
    m_renderMainTangents = shouldRender;
  }
}

void
rosy_gl_widget::renderOtherTangents(bool shouldRender) {
  if (m_renderOtherTangents != shouldRender) {
    m_renderOtherTangents = shouldRender;
  }
}

void
rosy_gl_widget::renderErrorColours(bool shouldRender) {
  if (m_renderErrorColours != shouldRender) {
    m_renderErrorColours = shouldRender;
  }
}

void rosy_gl_widget::select(const Eigen::Vector3f &camera_origin,
                            const Eigen::Vector3f &ray_direction) {
  using namespace std;
  using namespace Eigen;

  vector<Vector3f> items;
  for (int i = 0; i < m_positions.size(); i += 3) {
    items.emplace_back(m_positions[i],
                       m_positions[i + 1],
                       m_positions[i + 2]);
  }

  float distance;
  int selected_item = find_closest_vertex(camera_origin, ray_direction, items, distance);
  if (selected_item == -1) {
    m_selected_item_data.clear();
    m_neighbour_data.clear();
  }
  auto selected_vertex = items[selected_item];
  spdlog::info("Selected item index {} ({:2.2f}, {:2.2f}, {:2.2f})",
               selected_item,
               selected_vertex[0],
               selected_vertex[1],
               selected_vertex[2]);

  const auto w = window();
  ((field_visualiser_window *) w)->m_rosy_geometry_extractor->select_item_and_neighbours_at(selected_vertex,
                                                                                            m_selected_item_data,
                                                                                            m_neighbour_data);
}