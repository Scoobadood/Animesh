#pragma once

#include <QColor>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLDebugLogger>
#include <QOpenGLTexture>

#include <Surfel/SurfelGraph.h>
#include <ArcBall/ArcBall.h>
#include <vector>

#include "field_gl_widget.h"

class posy_gl_widget : public field_gl_widget {
Q_OBJECT
public:
    explicit posy_gl_widget(
            QWidget *parent = nullptr,
            Qt::WindowFlags f = Qt::WindowFlags());

    void setPoSyData(const std::vector<float> &positions,
                     const std::vector<float> &quads,
                     const std::vector<float> &triangle_fans,
                     const std::vector<float> &triangle_uvs,
                     const std::vector<unsigned int> &fan_sizes,
                     const std::vector<float> &normals,
                     const std::vector<float> &splat_sizes,
                     const std::vector<float> &uvs
    );

    void render_triangle_fans(bool render) {
        if (m_render_triangle_fans == render) {
            return;
        }
        m_render_triangle_fans = render;
        update();
    }

    void render_textures(bool render) {
        if (m_render_textures == render) {
            return;
        }
        m_render_textures = render;
        if (m_render_textures) {
            glEnable(GL_TEXTURE_2D);
            checkGLError("Enable tex 2D");
        } else {
            glDisable(GL_TEXTURE_2D);
            checkGLError("Disable tex 2D");
        }
        update();
    }

    void render_quads(bool render) {
        if (m_render_quads == render) {
            return;
        }
        m_render_quads = render;
        update();
    }

    void setRho(float rho) {
        if (m_rho != rho) {
            m_rho = rho;
            update();
        }
    }

protected:
    void initializeGL() override;

    void do_paint() override;
private:
    std::vector<float> m_positions;
    std::vector<float> m_quads;
    std::vector<float> m_triangle_fans;
    std::vector<float> m_triangle_uvs;
    std::vector<float> m_normals;
    std::vector<float> m_uvs;
    std::vector<int> m_fan_sizes;

    void drawPositions() const;

    void preRender(float &oldLineWidth) const;

    void postRender(float oldLineWidth) const;

    void maybeDrawQuads() const;

    void maybeDrawTriangleFans() const;

    static QImage generateTexture();

    QOpenGLTexture *m_texture;

    bool m_render_quads;
    bool m_render_textures;
    bool m_render_triangle_fans;
    std::vector<float> m_splat_sizes;
    float m_rho;
    float m_splat_scale_factor;
};
