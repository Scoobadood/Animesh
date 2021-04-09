#pragma once

#include <QColor>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <Surfel/SurfelGraph.h>
#include <vector>
#include <QOpenGLDebugLogger>

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
                     float scale_factor);

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

    inline void setZFar(float zFar) {
        if (m_z_far != zFar) {
            m_z_far = zFar;
            update();
        }
    }

    inline void setFov(float fov) {
        if( m_fov != fov) {
            m_fov = fov;
            update();
        }
    }

    /**
     * Rotate the camera about a point in front of it (m_target). Theta is a rotation
     * that tilts the camera forward and backward. Phi tilts the camera side to side.
     *
     * @param dTheta    The number of radians to rotate in the theta direction
     * @param dPhi      The number of radians to rotate in the phi direction
     */
    void rotate(float dTheta, float dPhi);

    /**
     * Move the camera down the look vector, closer to m_target. If we overtake m_target,
     * it is reprojected 30 units down the look vector
     *
     * TODO: Find a way to *not* hard-code the reprojection distance. Perhaps base it on the
     *       scene size? Or maybe have it defined in an settings.ini file
     *
     * @param distance    The distance to zoom. Negative distance will move the camera away from the target, positive will move towards
     */
    void zoom(float distance);

    /**
         * Moves the camera within its local X-Y plane
         *
         * @param dx    The amount to move the camera right or left
         * @param dy    The amount to move the camera up or down
         */
    void pan(float dx, float dy);

protected:
    void paintGL() override;
    void initializeGL() override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeGL(int width, int height) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    const float ROTATE_FACTOR = 300.0f;
    const float PAN_FACTOR = 0.01f;
    const float ZOOM_FACTOR = 0.1f;

    std::vector<float> m_positions;
    std::vector<float> m_tangents;
    std::vector<float> m_normals;
    float m_normal_scale_factor;
    bool m_renderNormals;
    bool m_renderMainTangents;
    bool m_renderOtherTangents;
    QColor m_normalColour;
    QColor m_mainTangentColour;
    QColor m_otherTangentsColour;

    void drawPositions() const;
    void maybeDrawNormals() const;
    void maybeDrawMainTangents() const;
    void maybeDrawOtherTangents() const;

    void setupDummyData();

    void clear();
    void applyModelViewMatrix();
    void applyProjectionMatrix();

    float m_fov;
    float m_z_near;
    float m_z_far;
    float m_aspect_ratio;
    QOpenGLDebugLogger * m_debugLogger;
    void checkGLError(const std::string& context) const;
    //
    // ArcBall Controls
    //
    float m_theta;
    float m_phi;
    float m_radius;
    float m_up;
    QPoint m_lastPixelPosition;
    QVector3D m_target;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelViewMatrix;
    bool m_modelViewMatrixIsDirty;
    QVector3D getCameraPosition() const;
    void updateModelViewMatrix();
    QVector3D toCartesian() const;

signals:
    void cameraPositionChanged(float x, float y, float z) const;
    void cameraOrientationChanged(float roll, float pitch, float yaw) const;
};
