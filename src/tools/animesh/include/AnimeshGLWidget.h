#pragma once

#include <QOpenGLWidget>
#include <ArcBall/AbstractArcBall.h>

class AnimeshGLWidget : public QOpenGLWidget {
 Q_OBJECT
 public:
  explicit AnimeshGLWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
  void set_arc_ball(const std::shared_ptr<AbstractArcBall> &arc_ball);

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
  static void draw_vertex_positions() ;
  void maybe_draw_normals() const;
  void maybe_draw_tangents() const;
  void maybe_draw_main_tangents() const;
  static void set_drawing_colour(const QColor &colour) ;
  static void checkGLError(const std::string &context) ;

  float m_fov;
  float m_z_near;
  float m_z_far;
  float m_aspect_ratio;
  bool m_projection_matrix_is_dirty;
  float m_projection_matrix[16];
  float m_model_view_matrix[16];
  std::shared_ptr<AbstractArcBall> m_arc_ball;

  QColor m_normal_colour;
  QColor m_tangent_colour;
  QColor m_main_tangent_colour;

  bool m_show_normals;
  bool m_show_tangents;
  bool m_show_main_tangents;
  float m_scale;

};
