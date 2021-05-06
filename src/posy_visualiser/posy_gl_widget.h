#pragma once

#include <QColor>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QOpenGLDebugLogger>
#include <QOpenGLTexture>

#include <Surfel/SurfelGraph.h>
#include <ArcBall/ArcBall.h>
#include <vector>

/**
 * @brief The posy_gl_widget class.
 * Renders PoSy with trackball controls.
 */
class posy_gl_widget : public QOpenGLWidget {
Q_OBJECT
public:
    explicit posy_gl_widget(
            QWidget *parent = nullptr,
            Qt::WindowFlags f = Qt::WindowFlags());

    void setPoSyData(const std::vector<float> &positions,
                     const std::vector<float> &quads,
                     const std::vector<float> &normals,
                     const std::vector<float> &splat_sizes,
                     const std::vector<float> &uvs
    );

    void renderQuads(bool render) {
        if (m_renderQuads == render) {
            return;
        }
        m_renderQuads = render;
        update();
    }

    void renderSplats(bool render) {
        if (m_renderSplats == render) {
            return;
        }
        m_renderSplats = render;
        update();
    }

    void setRho(float rho) {
        if( m_rho != rho) {
            m_rho = rho;
            update();
        }
    }

    void setZFar(float zFar);

    void setFov(float fov);

protected:
    void paintGL() override;

    void initializeGL() override;

    void resizeGL(int width, int height) override;

private:
    std::vector<float> m_positions;
    std::vector<float> m_quads;
    std::vector<float> m_normals;
    std::vector<float> m_uvs;

    ArcBall *m_arcBall;

    void maybeUpdateModelViewMatrix();

    void maybeUpdateProjectionMatrix() const;

    static void clear();

    void drawPositions() const;

    void maybeDrawSplats() const;

    void maybeDrawQuads() const;

    static QImage makeSplatImage() ;

    QOpenGLTexture *splatTexture;

    float m_fov;
    float m_zNear;
    float m_zFar;
    float m_aspectRatio;
    bool m_projectionMatrixIsDirty;
    bool m_renderSplats;
    bool m_renderQuads;
    std::vector<float> m_splat_sizes;
    float m_rho;
    float m_splat_scale_factor;

    static void checkGLError(const std::string &context);

signals:

    void cameraPositionChanged(float x, float y, float z) const;

    void cameraOrientationChanged(float roll, float pitch, float yaw) const;
};
