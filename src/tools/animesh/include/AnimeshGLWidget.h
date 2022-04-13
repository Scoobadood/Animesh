#pragma once

#include <QOpenGLWidget>
#include <ArcBall/AbstractArcBall.h>
#include <Surfel/SurfelGraph.h>
#include "AnimeshWindow.h"

class AnimeshGLWidget : public QOpenGLWidget {
 Q_OBJECT
 public:
  explicit AnimeshGLWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  void set_arc_ball(const std::shared_ptr<AbstractArcBall> &arc_ball);

  inline void toggle_surface() {
    m_show_surface = !m_show_surface;
    update();
  }

  inline void toggle_vertices() {
    m_show_vertices = !m_show_vertices;
    update();
  }

  inline void toggle_normals() {
    m_show_normals = !m_show_normals;
    update();
  }

  inline void toggle_tangents(){
    m_show_tangents = !m_show_tangents;
    update();
  }

  inline void toggle_main_tangents() {
    m_show_main_tangents = !m_show_main_tangents;
    update();
  }

  inline void toggle_posy_vertices() {
    m_show_posy_vertices = !m_show_posy_vertices;
    update();
  }

  inline void toggle_consensus_graph() {
    m_show_consensus_graph = !m_show_consensus_graph;
    update();
  }

  inline void set_scale( float scale ) {
    if( m_scale == scale ) {
      return;
    }

    m_scale = scale;
    update();
  }

 protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;

 private:
  static void clear() ;
  void update_model_matrix();
  void maybe_update_projection_matrix();
  void maybe_draw_vertex_positions() const ;
  void maybe_draw_surface() ;
  void maybe_draw_normals() const;
  void maybe_draw_tangents() const;
  void maybe_draw_main_tangents() const;
  void maybe_draw_posy_vertices() const;
  void maybe_draw_consensus_graph() const;
  void generate_surface(const SurfelGraphPtr &graph);


  static void set_drawing_colour(const QColor &colour) ;
  static void checkGLError(const std::string &context) ;
  AnimeshWindow * get_main_window() const;

  float m_fov;
  float m_z_near;
  float m_z_far;
  float m_aspect_ratio;
  bool m_projection_matrix_is_dirty;
  float m_projection_matrix[16];
  float m_model_view_matrix[16];
  std::shared_ptr<AbstractArcBall> m_arc_ball;

  QColor m_vertex_colour;
  QColor m_normal_colour;
  QColor m_tangent_colour;
  QColor m_main_tangent_colour;
  QColor m_posy_vertex_colour;
  QColor m_posy_surface_colour;

  bool m_show_vertices;
  bool m_show_surface;
  bool m_show_normals;
  bool m_show_tangents;
  bool m_show_main_tangents;
  bool m_show_posy_vertices;
  bool m_show_consensus_graph;
  std::vector<float> m_surface_faces;
  float m_scale;

};
