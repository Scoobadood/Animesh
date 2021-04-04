#ifndef POSY_GL_WIDGET_H
#define POSY_GL_WIDGET_H

#include <QColor>
#include <QOpenGLWidget>
#include <Surfel/SurfelGraph.h>
#include <vector>

/**
 * @brief The rosy_gl_widget class.
 * Renders RoSy with trackball controls and allows selectoin of individual surfels.
 * Interface contract:
 * Can accept new data and redraw
 * Can handle mouse selection of a surfel and emits a selected event
 * Can locally manage orientation of the view.
 * Maintains state about what to render (normals, tangents, etc.)
 */
class rosy_gl_widget : public QOpenGLWidget {
    Q_OBJECT
public:
    rosy_gl_widget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void setRoSyData(const std::vector<float>& positions,
                     const std::vector<float>& normals,
                     const std::vector<float>& tangents,
                     const float scale_factor);

    inline void renderNormals( bool shouldRender) {
        if( m_renderNormals != shouldRender) {
            m_renderNormals = shouldRender;
            update();
        }
    }

    inline void renderMainTangents( bool shouldRender) {
        if( m_renderMainTangents != shouldRender) {
            m_renderMainTangents = shouldRender;
            update();
        }
    }

    inline void renderOtherTangents( bool shouldRender) {
        if( m_renderOtherTangents != shouldRender) {
            m_renderOtherTangents = shouldRender;
            update();
        }
    }

protected:
    void paintGL() override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    std::vector<float> m_positions;
    std::vector<float> m_tangents;
    std::vector<float> m_normals;
    float m_normal_scale_factor;
    int m_frame;
    bool m_renderNormals;
    bool m_renderMainTangents;
    bool m_renderOtherTangents;
    bool m_isDirty;
    QColor m_normalColour;
    QColor m_mainTangentColour;
    QColor m_otherTangentsColour;

    void maybeDrawNormals() const;
    void maybeDrawMainTangents() const;
    void maybeDrawOtherTangents() const;

    void setupDummyData();

    void clear();
    void setModelViewMatrix();
    void setProjectionMatrix();

    float m_fov;
    float m_front;
    float m_back;
    float m_aspect_ratio;

    float m_camera_x;
    float m_camera_y;
    float m_camera_z;

    float m_camera_pitch;
    float m_camera_yaw;
    float m_camera_roll;
};

#endif // POSY_GL_WIDGET_H
