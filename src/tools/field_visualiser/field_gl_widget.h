//
// Created by Dave Durbin (Old) on 28/6/21.
//

#pragma once

#include <Eigen/Core>
#include <QOpenGLWidget>
#include <ArcBall/ArcBall.h>

class field_gl_widget : public QOpenGLWidget {
Q_OBJECT
public:
  explicit field_gl_widget(
      QWidget *parent = nullptr,
      Qt::WindowFlags f = Qt::WindowFlags());

  void set_arc_ball(const std::shared_ptr<AbstractArcBall> &arc_ball);

protected:
  void paintGL() override;

  virtual void do_paint() = 0;

  void initializeGL() override = 0;

  void resizeGL(int width, int height) override;

  static void checkGLError(const std::string &context);

  static void clear();
  int find_closest_edge(unsigned int pixel_x,
                        unsigned int pixel_y,
                        std::vector<std::pair<Eigen::Vector3f, Eigen::Vector3f>> &edges,
                        float &distance);

  void enable_light() {
    m_light_enabled = true;
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
  }

  void maybe_update_light();

private:
  int find_closest_vertex(unsigned int pixel_x,
                          unsigned int pixel_y,
                          std::vector<float> &items, /// assumed XYZ triples
                          float &distance);
  void get_ray_data(unsigned int pixel_x,
                    unsigned int pixel_y,
                    Eigen::Vector3f &camera_origin,
                    Eigen::Vector3f &ray_direction);

  std::shared_ptr<AbstractArcBall> m_arc_ball;

  float m_fov;
  float m_zNear;
  float m_zFar;
  float m_aspectRatio;
  bool m_projectionMatrixIsDirty;
  float m_projection_matrix[16];
  float m_model_view_matrix[16];

  Eigen::Vector3f m_near_point;
  Eigen::Vector3f m_far_point;
  bool m_render_mouse_ray;
  bool m_light_enabled;
  void maybe_render_mouse_ray();

  void update_model_matrix();
  Eigen::Vector3f ray_direction_for_pixel(int pixel_x, int pixel_y);

  void maybe_update_projection_matrix();
};