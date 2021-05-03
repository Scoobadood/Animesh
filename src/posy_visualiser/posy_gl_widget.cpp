#include "posy_gl_widget.h"

#include <vector>
#include <QColor>
#include <QImage>
#include <Surfel/SurfelGraph.h>

const float DEG2RAD = (3.14159265f / 180.0f);

posy_gl_widget::posy_gl_widget(
        QWidget *parent, Qt::WindowFlags f) :
        QOpenGLWidget{parent, f} //
        , m_fov{60} //
        , m_zNear{0.5f} //
        , m_zFar{50.0f} //
        , m_aspectRatio{1.0f} //
        , m_projectionMatrixIsDirty{true} //
        , m_renderSplats{true} //
        , m_renderQuads{true} //
        , m_splatSize{0.2f} //
        , m_rho{1.0f} //
{
    m_arcBall = new ArcBall();
    installEventFilter(m_arcBall);
    setFocus();
    // Dummy data
    setPoSyData(
            std::vector<float>{0.0f, 0.0f, 0.0f},
            std::vector<float>{-0.4f, 0.0f, -0.4f,
                               -0.4, 0.0, 0.4f,
                               0.4f, 0.0f, 0.4f,
                               0.4f, 0.0f, -0.4f},
            std::vector<float>{0.0f, 1.0f, 0.0f},
            std::vector<float>{0.0f, 0.0f}
    );
}

void
posy_gl_widget::maybeDrawSplats() const {
    if (!m_renderSplats) {
        return;
    }
    glEnable(GL_TEXTURE_2D);
    checkGLError("Enable tex 2D");

    glBindTexture(GL_TEXTURE_2D, splatTexture->textureId());
    checkGLError("bound texture");

    const auto numPositions = m_positions.size() / 3;
    glColor4d(1.0, 1.0, 1.0, 1.0);

    for (int i = 0; i < numPositions; ++i) {
        glBegin(GL_QUADS);
        glNormal3d(m_normals.at(i * 3 + 0),
                   m_normals.at(i * 3 + 1),
                   m_normals.at(i * 3 + 2));

        // S and t are in range [-0.5, 0.5)
        auto s = m_uvs.at(i * 2 + 0);
        auto t = m_uvs.at(i * 2 + 1);


        s = -(m_splatSize * 0.5f) - s;
        t = -(m_splatSize * 0.5f) - t;

        // uvs are in the range 0 -> 1
        // but splat_size maybe 0.1 ... 1.0
        // Rho is arbitrarily large
        // We alrady have quads sized to splatSize
        // and UV points to the right part of the texture (s,t are fine)
        // but the *proportion* of the texture to render will vary with splat size
        float tileSize = m_splatSize;
        glTexCoord2d(s, t);
        glVertex3f(m_quads.at(i * 12 + 0),
                   m_quads.at(i * 12 + 1),
                   m_quads.at(i * 12 + 2));

        glTexCoord2d(s, t + tileSize);
        glVertex3f(m_quads.at(i * 12 + 3),
                   m_quads.at(i * 12 + 4),
                   m_quads.at(i * 12 + 5));

        glTexCoord2d(s + tileSize, t + tileSize);
        glVertex3f(m_quads.at(i * 12 + 6),
                   m_quads.at(i * 12 + 7),
                   m_quads.at(i * 12 + 8));

        glTexCoord2d(s + tileSize, t);
        glVertex3f(m_quads.at(i * 12 + 9),
                   m_quads.at(i * 12 + 10),
                   m_quads.at(i * 12 + 11));
        glEnd();
    }
    glFlush();
    glDisable(GL_TEXTURE_2D);
    checkGLError("maybeDrawSplats");
}

void
posy_gl_widget::maybeDrawQuads() const {
    if (!m_renderQuads) {
        return;
    }
    const auto numPositions = m_positions.size() / 3;
    glColor4d(1.0, 1.0, 1.0, 1.0);

    for (int i = 0; i < numPositions; ++i) {
        glBegin(GL_LINE_LOOP);
        glNormal3d(m_normals.at(i * 3 + 0),
                   m_normals.at(i * 3 + 1),
                   m_normals.at(i * 3 + 2));

        glVertex3f(m_quads.at(i * 12 + 0),
                   m_quads.at(i * 12 + 1),
                   m_quads.at(i * 12 + 2));

        glVertex3f(m_quads.at(i * 12 + 3),
                   m_quads.at(i * 12 + 4),
                   m_quads.at(i * 12 + 5));

        glVertex3f(m_quads.at(i * 12 + 6),
                   m_quads.at(i * 12 + 7),
                   m_quads.at(i * 12 + 8));

        glVertex3f(m_quads.at(i * 12 + 9),
                   m_quads.at(i * 12 + 10),
                   m_quads.at(i * 12 + 11));
        glEnd();
    }
    glFlush();
    checkGLError("maybeDrawQuads");
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
posy_gl_widget::clear() {
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGLError("clear");
}

/**
 * If the ModelView matrix is dirty, update and reload it.
 */
void
posy_gl_widget::maybeUpdateModelViewMatrix() {
    if (m_arcBall->modelViewMatrixHasChanged()) {
        glMatrixMode(GL_MODELVIEW);
        float m[16];
        m_arcBall->modelViewMatrix(m);
        glLoadMatrixf(m);
    }
    checkGLError("maybeUpdateModelViewMatrix");
}

/**
 * If the projection matrix is dirty, update and reload it.
 */
void
posy_gl_widget::maybeUpdateProjectionMatrix() const {
    if (m_projectionMatrixIsDirty) {
        const auto yMax = tan(m_fov * DEG2RAD * 0.5f);
        const auto xMax = yMax * m_aspectRatio;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-xMax, xMax, -yMax, yMax, m_zNear, m_zFar);
    }
    checkGLError("maybeUpdateProjectionMatrix");
}

/**
 * Resize the viewport when the window resizes.
 */
void
posy_gl_widget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    m_aspectRatio = (float) width / (float) height;
    m_projectionMatrixIsDirty = true;
}

void
posy_gl_widget::paintGL() {
    clear();

    maybeUpdateProjectionMatrix();

    maybeUpdateModelViewMatrix();

    drawPositions();

    maybeDrawSplats();

    maybeDrawQuads();
}

void
posy_gl_widget::setPoSyData(const std::vector<float> &positions,
                            const std::vector<float> &quads,
                            const std::vector<float> &normals,
                            const std::vector<float> &uvs
) {
    m_positions.clear();
    m_quads.clear();
    m_normals.clear();
    m_uvs.clear();

    m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
    m_quads.insert(m_quads.begin(), quads.begin(), quads.end());
    m_normals.insert(m_normals.begin(), normals.begin(), normals.end());
    m_uvs.insert(m_uvs.begin(), uvs.begin(), uvs.end());

    update();
}

void
posy_gl_widget::checkGLError(const std::string &context) {
    auto err = glGetError();
    if (!err) return;
    spdlog::error("{}: {} ", context, err);
}

QImage
posy_gl_widget::makeSplatImage() const {
    QImage img(64, 64, QImage::Format_ARGB32);
    for (int x = -31; x < 32; x++) {
        for (int y = -31; y < 32; y++) {
            float d = std::sqrtf(x * x + y * y);
            int a = std::max<int>(0, 255 - (int) (d * 8));
            a = 255;
            int r, g, b;
            r = g = b = (x < -30 || y < -30 || x > 30 || y >30 )
                        ? 0
                        : 127;
            int colour = (a << 24) | (r << 16) | (g << 8) | b;
            img.setPixel(x + 31, y + 31, colour);
        }
    }
    checkGLError("Making image");
    return img;
}

void
posy_gl_widget::initializeGL() {
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto img = makeSplatImage();
    splatTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
    splatTexture->setData(img, QOpenGLTexture::GenerateMipMaps);
    splatTexture->setMinMagFilters(QOpenGLTexture::LinearMipMapLinear, QOpenGLTexture::Nearest);
    splatTexture->setWrapMode(QOpenGLTexture::Repeat);
    checkGLError("Generating texture");
}

void
posy_gl_widget::setZFar(float zFar) {
    if (m_zFar != zFar) {
        m_zFar = zFar;
        m_projectionMatrixIsDirty = true;
        update();
    }
}

void
posy_gl_widget::setFov(float fov) {
    if (m_fov != fov) {
        m_fov = fov;
        m_projectionMatrixIsDirty = true;
        update();
    }
}