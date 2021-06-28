#include "posy_gl_widget.h"

#include <vector>
#include <QColor>
#include <QImage>

posy_gl_widget::posy_gl_widget(QWidget *parent, Qt::WindowFlags f) //
        : field_gl_widget{parent, f} //
        , m_render_quads{true} //
        , m_render_textures{false} //
        , m_render_triangle_fans{false} //
        , m_rho{1.0f} //
{
    setFocus();
    // Dummy data
    setPoSyData(
            std::vector<float>{0.0f, 0.0f, 0.0f},
            std::vector<float>{-0.4f, 0.0f, -0.4f,
                               -0.4, 0.0, 0.4f,
                               0.4f, 0.0f, 0.4f,
                               0.4f, 0.0f, -0.4f},
            std::vector<float>{ //
                    0.0f, 0.0f, 0.0f, // hub
                    0.4f, 0.0f, -0.4f,
                    0.4f, 0.0f, 0.4f,
                    -0.4, 0.0, 0.4f,
                    -0.4f, 0.0f, -0.4f,
                    0.4f, 0.0f, -0.4f,
            },
            std::vector<float>{
                    0.5f, 0.5f,
                    1.0f, 0.0f,
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    0.0f, 0.0f,
                    1.0f, 0.0f
            },
            std::vector<unsigned int>{6},
            std::vector<float>{0.0f, 1.0f, 0.0f},
            std::vector<float>{1.0f},
            std::vector<float>{0.0f, 0.0f}
    );
}

void
posy_gl_widget::preRender(float &oldLineWidth) const {
    if (m_render_textures) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_TEXTURE_2D);
        m_texture->bind();
        checkGLError("bind texture");
    } else {
        glGetFloatv(GL_LINE_WIDTH, &oldLineWidth);
        glPolygonMode(GL_FRONT, GL_LINE);
        glLineWidth(3.0f);
    }
}

void
posy_gl_widget::postRender(float oldLineWidth) const {
    if (m_render_textures) {
        m_texture->release();
        glDisable(GL_TEXTURE_2D);
        checkGLError("unbind texture");
    } else {
        glLineWidth(oldLineWidth);
    }
}

void
posy_gl_widget::maybeDrawQuads() const {
    if (!m_render_quads) {
        return;
    }

    float oldLineWidth;
    preRender(oldLineWidth);

    const auto numPositions = m_positions.size() / 3;

    // All vertices are white
    glColor4d(1.0, 1.0, 1.0, 1.0);

    for (int i = 0; i < numPositions; ++i) {
        glBegin(GL_QUADS);
        glNormal3d(m_normals.at(i * 3 + 0),
                   m_normals.at(i * 3 + 1),
                   m_normals.at(i * 3 + 2));

        // S and t are in range [-0.5, 0.5)
        const auto u = m_uvs.at(i * 2 + 0);
        const auto v = m_uvs.at(i * 2 + 1);
        const auto splat_size = m_splat_sizes.at(i);

        const auto s = u - (splat_size * 0.5f);
        const auto t = v - (splat_size * 0.5f);

        glTexCoord2d(s, t);
        glVertex3f(m_quads.at(i * 12 + 0),
                   m_quads.at(i * 12 + 1),
                   m_quads.at(i * 12 + 2));

        glTexCoord2d(s, t + splat_size);
        glVertex3f(m_quads.at(i * 12 + 3),
                   m_quads.at(i * 12 + 4),
                   m_quads.at(i * 12 + 5));

        glTexCoord2d(s + splat_size, t + splat_size);
        glVertex3f(m_quads.at(i * 12 + 6),
                   m_quads.at(i * 12 + 7),
                   m_quads.at(i * 12 + 8));

        glTexCoord2d(s + splat_size, t);
        glVertex3f(m_quads.at(i * 12 + 9),
                   m_quads.at(i * 12 + 10),
                   m_quads.at(i * 12 + 11));
        glEnd();
    }
    glFlush();
    postRender(oldLineWidth);
    checkGLError("maybeDrawQuads");
}

/*
 * Each vertex has a triangle fan associated with it.
 * We just draw the fans with no texture initially
 */

GLubyte cols[] = {
        0xE6, 0x9F, 0x00, 0x99, 0x99, 0x99, 0xcc, 0x79, 0xa7, 0xd5, 0x5e, 0x00, 0x00, 0x72, 0xb2, 0xf0, 0xe4, 0x42,
        0x00, 0x9e, 0x73, 0x56, 0xb4, 0xe9
};

void
posy_gl_widget::maybeDrawTriangleFans() const {
    if (!m_render_triangle_fans) {
        return;
    }
    float oldLineWidth;
    preRender(oldLineWidth);
    unsigned int fan_index = 0;
    for (int fan_size : m_fan_sizes) {
        glBegin(GL_TRIANGLE_FAN);

        // Hub vertex then (fan_size+1) additional vertices
        glColor4ub(255, 255, 255, 255);
        for (auto j = 0; j < fan_size; ++j) {
            glTexCoord2d(m_triangle_uvs.at((fan_index / 3) * 2 + 0),
                         m_triangle_uvs.at((fan_index / 3) * 2 + 1));
            glVertex3f(m_triangle_fans.at(fan_index + 0),
                       m_triangle_fans.at(fan_index + 1),
                       m_triangle_fans.at(fan_index + 2));
            fan_index += 3;
        }
        glEnd();
    }
    glFlush();
    postRender(oldLineWidth);
    checkGLError("maybeDrawTriangleFans");
}

void
posy_gl_widget::drawPositions() const {
    glColor4d(1.0, 0.0, 0.0, 1.0);

    glEnable(GL_POINT_SMOOTH);
    float oldPointSize;
    glGetFloatv(GL_POINT_SIZE, &oldPointSize);

    glPointSize(5.0f);
    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        glBegin(GL_POINTS);
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glEnd();
    }
    glPointSize(oldPointSize);
    glDisable(GL_POINT_SMOOTH);
    checkGLError("drawPositions");
}

void
posy_gl_widget::do_paint() {
    drawPositions();

    maybeDrawQuads();

    maybeDrawTriangleFans();
}

void
posy_gl_widget::setPoSyData(const std::vector<float> &positions,
                            const std::vector<float> &quads,
                            const std::vector<float> &triangle_fans,
                            const std::vector<float> &triangle_uvs,
                            const std::vector<unsigned int> &fan_sizes,
                            const std::vector<float> &normals,
                            const std::vector<float> &splat_sizes,
                            const std::vector<float> &uvs
) {
    m_positions.clear();
    m_quads.clear();
    m_triangle_fans.clear();
    m_triangle_uvs.clear();
    m_fan_sizes.clear();
    m_normals.clear();
    m_splat_sizes.clear();
    m_uvs.clear();

    m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
    m_quads.insert(m_quads.begin(), quads.begin(), quads.end());
    m_triangle_fans.insert(m_triangle_fans.begin(), triangle_fans.begin(), triangle_fans.end());
    m_triangle_uvs.insert(m_triangle_uvs.begin(), triangle_uvs.begin(), triangle_uvs.end());
    m_fan_sizes.insert(m_fan_sizes.begin(), fan_sizes.begin(), fan_sizes.end());
    m_normals.insert(m_normals.begin(), normals.begin(), normals.end());
    m_splat_sizes.insert(m_splat_sizes.begin(), splat_sizes.begin(), splat_sizes.end());
    m_uvs.insert(m_uvs.begin(), uvs.begin(), uvs.end());
}

QImage
posy_gl_widget::generateTexture() {
    QImage img(500, 500, QImage::Format_ARGB32);
    unsigned int colour;
    for (int x = 0; x < 500; ++x) {
        for (int y = 0; y < 500; ++y) {
            if (x < 20 || y < 20) {
                colour = 0xFF942C20;
            } else {
                colour = 0xFFFFFFFF;
            }
            img.setPixel(x, y, colour);
        }
    }
    checkGLError("Making image");
    return img;
}

void
posy_gl_widget::initializeGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto img = generateTexture();
    m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    m_texture->setData(img, QOpenGLTexture::GenerateMipMaps);
    m_texture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Nearest);
    m_texture->setWrapMode(QOpenGLTexture::Repeat);
    checkGLError("Generating texture");
}

