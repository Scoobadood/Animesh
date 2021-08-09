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

  void set_arc_ball(ArcBall *arc_ball);

protected:
  void paintGL() override;

  virtual void do_paint() = 0;

  void initializeGL() override = 0;

  void resizeGL(int width, int height) override;

  static void checkGLError(const std::string &context);

  static void clear();
  int find_closest_vertex(unsigned int pixel_x,
                          unsigned int pixel_y,
                          std::vector<float> &items, /// assumed XYZ triples
                          float &distance);
private:
  ArcBall *m_arcBall;

  float m_fov;
  float m_zNear;
  float m_zFar;
  float m_aspectRatio;
  bool m_projectionMatrixIsDirty;
  float m_projection_matrix[16];
  float m_model_view_matrix[16];

  void update_model_matrix();
  Eigen::Vector3f ray_for_pixel(int pixel_x, int pixel_y);

  void maybe_update_projection_matrix();
};