//
// Created by Dave Durbin (Old) on 28/6/21.
//

#pragma once


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


  int
  find_closest_vertex(unsigned int pixel_x,
                      unsigned int pixel_y,
                      std::vector<Eigen::Vector3f>& items,
                      float& distance);
private:
    ArcBall *m_arcBall;

    float m_fov;
    float m_zNear;
    float m_zFar;
    float m_aspectRatio;
    bool m_projectionMatrixIsDirty;
    void update_model_matrix();

    void maybe_update_projection_matrix() const;
};