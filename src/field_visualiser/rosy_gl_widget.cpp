#include "rosy_gl_widget.h"

#include <vector>
#include <QColor>

rosy_gl_widget::rosy_gl_widget(
        QWidget *parent, //
        Qt::WindowFlags f) //
        : field_gl_widget{parent, f}
        , m_normalScaleFactor{1.0f} //
        , m_renderNormals{true} //
        , m_renderMainTangents{true} //
        , m_renderOtherTangents{true} //
        , m_renderErrorColours{true} //
        , m_normalColour{QColor{255, 255, 255, 255}} //
        , m_mainTangentColour{QColor{255, 0, 0, 255}} //
        , m_otherTangentsColour{QColor{0, 255, 0, 255}} //
{
    setFocus();
    // Dummy data
    setRoSyData(
            std::vector<float>{0.0f, 0.0f, 0.0f},
            std::vector<float>{0.0f, 1.0f, 0.0f},
            std::vector<float>{0.0f, 0.0f, 1.0f},
            std::vector<float>{0.0f, 1.0f, 0.0f, 1.0f},
            1.0f
    );
}

void
rosy_gl_widget::maybeDrawNormals() const {
    if (!m_renderNormals) {
        return;
    }
    glColor4d(m_normalColour.redF(), m_normalColour.greenF(),
              m_normalColour.blueF(), m_normalColour.alphaF());

    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        if( m_renderErrorColours) {
            auto r = m_colours.at(i * 4 + 0);
            auto g = m_colours.at(i * 4 + 1);
            auto b = m_colours.at(i * 4 + 2);
            auto a = m_colours.at(i * 4 + 3);
            glColor4f(r, g, b, a);
        }
        glBegin(GL_LINES);
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glVertex3f(m_positions.at(i * 3 + 0) + (m_normals.at(i * 3 + 0) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 1) + (m_normals.at(i * 3 + 1) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 2) + (m_normals.at(i * 3 + 2) * m_normalScaleFactor));
        glEnd();
    }
    checkGLError("maybeDrawNormals");

}

void
rosy_gl_widget::drawPositions() const {
    glColor4d(m_normalColour.redF(),m_normalColour.greenF(),
              m_normalColour.blueF(),m_normalColour.alphaF());

    glEnable(GL_POINT_SMOOTH);
    float oldPointSize;
    glGetFloatv(GL_POINT_SIZE, &oldPointSize);

    glPointSize(3.0f);
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
rosy_gl_widget::maybeDrawMainTangents() const {
    if (!m_renderMainTangents) {
        return;
    }
        glColor4d(m_mainTangentColour.redF(),m_mainTangentColour.greenF(),
                  m_mainTangentColour.blueF(),m_mainTangentColour.alphaF());
    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        if( m_renderErrorColours) {
            auto r = m_colours.at(i * 4 + 0);
            auto g = m_colours.at(i * 4 + 1);
            auto b = m_colours.at(i * 4 + 2);
            auto a = m_colours.at(i * 4 + 3);
            glColor4f(r, g, b, a);
        }

        glBegin(GL_LINES);
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glVertex3f(m_positions.at(i * 3 + 0) + (m_tangents.at(i * 3 + 0) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 1) + (m_tangents.at(i * 3 + 1) * m_normalScaleFactor),
                   m_positions.at(i * 3 + 2) + (m_tangents.at(i * 3 + 2) * m_normalScaleFactor));
        glEnd();
    }
    checkGLError("maybeDrawTangents");

}

void
rosy_gl_widget::maybeDrawOtherTangents() const {
    if (!m_renderOtherTangents) {
        return;
    }
    glColor4f((GLfloat) m_otherTangentsColour.redF(),
              (GLfloat) m_otherTangentsColour.greenF(),
              (GLfloat) m_otherTangentsColour.blueF(),
              (GLfloat) m_otherTangentsColour.alphaF());

    for (unsigned int i = 0; i < m_positions.size() / 3; ++i) {
        if( m_renderErrorColours) {
            auto r = m_colours.at(i * 4 + 0);
            auto g = m_colours.at(i * 4 + 1);
            auto b = m_colours.at(i * 4 + 2);
            auto a = m_colours.at(i * 4 + 3);
            glColor4f(r, g, b, a);
        }
        // Get perpendicular tangent by computing cross(norm,tan)
        const auto normX = m_normals.at(i * 3 + 0);
        const auto normY = m_normals.at(i * 3 + 1);
        const auto normZ = m_normals.at(i * 3 + 2);
        const auto tanX = m_tangents.at(i * 3 + 0);
        const auto tanY = m_tangents.at(i * 3 + 1);
        const auto tanZ = m_tangents.at(i * 3 + 2);

        auto crossTanX = (normY * tanZ - normZ * tanY) * m_normalScaleFactor; //bn -cm
        auto crossTanY = (normZ * tanX - normX * tanZ) * m_normalScaleFactor; //bn -cm
        auto crossTanZ = (normX * tanY - normY * tanX) * m_normalScaleFactor; //bn -cm

        glBegin(GL_LINES);
        glVertex3f(m_positions.at(i * 3 + 0) - crossTanX,
                   m_positions.at(i * 3 + 1) - crossTanY,
                   m_positions.at(i * 3 + 2) - crossTanZ);
        glVertex3f(m_positions.at(i * 3 + 0) + crossTanX,
                   m_positions.at(i * 3 + 1) + crossTanY,
                   m_positions.at(i * 3 + 2) + crossTanZ);
        glVertex3f(m_positions.at(i * 3 + 0) - (tanX * m_normalScaleFactor),
                   m_positions.at(i * 3 + 1) - (tanY * m_normalScaleFactor),
                   m_positions.at(i * 3 + 2) - (tanZ * m_normalScaleFactor));
        glVertex3f(m_positions.at(i * 3 + 0),
                   m_positions.at(i * 3 + 1),
                   m_positions.at(i * 3 + 2));
        glEnd();
    }
    checkGLError("maybeDrawOtherTangents");
}


void
rosy_gl_widget::do_paint() {
    drawPositions();

    maybeDrawNormals();

    maybeDrawMainTangents();

    maybeDrawOtherTangents();
}

void
rosy_gl_widget::setRoSyData(const std::vector<float> &positions,
                            const std::vector<float> &normals,
                            const std::vector<float> &tangents,
                            const std::vector<float> &colours,
                            const float normal_scale_factor) {
    m_positions.clear();
    m_tangents.clear();
    m_normals.clear();
    m_colours.clear();

    m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
    m_tangents.insert(m_tangents.begin(), tangents.begin(), tangents.end());
    m_normals.insert(m_normals.begin(), normals.begin(), normals.end());
    m_colours.insert(m_colours.begin(), colours.begin(), colours.end());
    m_normalScaleFactor = normal_scale_factor;

    update();
}

void
rosy_gl_widget::initializeGL() {
    glEnable(GL_DEPTH);
    glLineWidth(3.0f);
}

void
rosy_gl_widget::renderNormals( bool shouldRender) {
    if( m_renderNormals != shouldRender) {
        m_renderNormals = shouldRender;
        update();
    }
}

void
rosy_gl_widget::renderMainTangents( bool shouldRender) {
    if( m_renderMainTangents != shouldRender) {
        m_renderMainTangents = shouldRender;
        update();
    }
}

void
rosy_gl_widget::renderOtherTangents( bool shouldRender) {
    if( m_renderOtherTangents != shouldRender) {
        m_renderOtherTangents = shouldRender;
        update();
    }
}

void
rosy_gl_widget::renderErrorColours( bool shouldRender) {
    if( m_renderErrorColours != shouldRender) {
        m_renderErrorColours = shouldRender;
        update();
    }
}