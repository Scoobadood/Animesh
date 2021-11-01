//
// Created by Dave Durbin (Old) on 28/6/21.
//

#pragma once

#include <QOpenGLWidget>
#include "field_gl_widget.h"

class quad_gl_widget : public field_gl_widget {
Q_OBJECT
public:
  explicit quad_gl_widget(
      QWidget *parent = nullptr,
      Qt::WindowFlags f = Qt::WindowFlags());

  void setData(
      const std::vector<float> &vertices,
      const std::vector<std::pair<std::pair<std::string,unsigned int>, std::pair<std::string,unsigned int>>> &red_edges,
      const std::vector<std::pair<std::pair<std::string,unsigned int>, std::pair<std::string,unsigned int>>> &blue_edges,
      const std::vector<float>& original_vertices,
      const std::vector<float>& vertex_affinity);

  inline void showBlueEdges(bool should_show) {
    if (m_show_blue_edges != should_show) {
      m_show_blue_edges = should_show;
      update();
    }
  }
  inline void showRedEdges(bool should_show) {
    if (m_show_red_edges != should_show) {
      m_show_red_edges = should_show;
      update();
    }
  }
  inline void showVertexAffinities(bool should_show) {
    if (m_show_vertex_affinities != should_show) {
      m_show_vertex_affinities = should_show;
      update();
    }
  }
protected:
  void do_paint() override;

  void initializeGL() override;

private:
  bool m_show_blue_edges;
  bool m_show_red_edges;
  bool m_show_vertex_affinities;

  std::vector<float> m_vertices;
  std::vector<std::pair<std::pair<std::string,unsigned int>, std::pair<std::string,unsigned int>>> m_red_edges;
  std::vector<std::pair<std::pair<std::string,unsigned int>, std::pair<std::string,unsigned int>>> m_blue_edges;
  std::vector<std::pair<Eigen::Vector3f, Eigen::Vector3f>> m_all_edges;

  std::vector<float> m_original_vertices;
  std::vector<float> m_vertex_affinity;

  void drawVertices() const;
  void maybe_draw_red_edges() const;
  void maybe_draw_blue_edges() const;
  void maybe_draw_selected_edge() const;
  void maybe_draw_original_vertex_affinities() const;

  void mouse_moved(unsigned int pixel_x, unsigned int pixel_y);
  void mouseMoveEvent(QMouseEvent *event) override;

  int m_selected_vertex = -1;
  int m_selected_edge = -1;

signals:
  void no_edge_selected() const ;
  void edge_selected(std::string& from_name, std::string& to_name) const ;
};