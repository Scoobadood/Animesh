#pragma once

#include <QColor>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>

#include <Surfel/SurfelGraph.h>
#include <arc_ball.h>
#include <vector>

/**
 * @brief The rosy_gl_widget class.
 * Renders RoSy with trackball controls and allows selection of individual surfels.
 * Interface contract:
 * Can accept new data and redraw
 * Can handle mouse selection of a surfel and emits a selected event
 * Can locally manage orientation of the view.
 * Maintains state about what to render (normals, tangents, etc.)
 */
class rosy_gl_widget : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit rosy_gl_widget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    void setRoSyData(const std::vector<float>& positions,
                     const std::vector<float>& normals,
                     const std::vector<float>& tangents,
                     float scale_factor);

    void renderNormals( bool shouldRender);
    void renderMainTangents( bool shouldRender);
    void renderOtherTangents( bool shouldRender);
    void setZFar(float zFar);
    void setFov(float fov);

protected:
    void paintGL() override;
    void initializeGL() override;
    void resizeGL(int width, int height) override;

private:
    std::vector<float> m_positions;
    std::vector<float> m_tangents;
    std::vector<float> m_normals;
    float m_normalScaleFactor;
    bool m_renderNormals;
    bool m_renderMainTangents;
    bool m_renderOtherTangents;
    QColor m_normalColour;
    QColor m_mainTangentColour;
    QColor m_otherTangentsColour;

    arc_ball * m_arcBall;

    void maybeUpdateModelViewMatrix();
    void maybeUpdateProjectionMatrix() const;
    static void clear();
    void drawPositions() const;
    void maybeDrawNormals() const;
    void maybeDrawMainTangents() const;
    void maybeDrawOtherTangents() const;

    float m_fov;
    float m_zNear;
    float m_zFar;
    float m_aspectRatio;
    bool m_projectionMatrixIsDirty;

    static void checkGLError(const std::string& context) ;

signals:
    void cameraPositionChanged(float x, float y, float z) const;
    void cameraOrientationChanged(float roll, float pitch, float yaw) const;
};
