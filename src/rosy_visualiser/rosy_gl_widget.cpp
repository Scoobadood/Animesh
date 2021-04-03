#include "rosy_gl_widget.h"

#include "QOpenGLFunctions"
#include <iostream>
#include <Surfel/SurfelGraph.h>
#include <vector>
#include <QColor>
#include <QKeyEvent>
#include <Geom/Geom.h>

const double DEG2RAD = 3.14159265 / 180;

rosy_gl_widget::rosy_gl_widget(QWidget* parent, Qt::WindowFlags f) :
    QOpenGLWidget{parent, f},
    m_renderNormals{true},
    m_renderMainTangents{true},
    m_renderOtherTangents{true},
    m_isDirty{false},
    m_normalColour{QColor{0, 0, 255, 255}},
    m_mainTangentColour{QColor{0, 255, 0, 255}},
    m_otherTangentsColour{QColor{255, 0, 255, 255}},
    m_fov{135},
    m_front{0.5f},
    m_back{10.0f},
    m_aspect_ratio{1.0f},
    m_camera_x{0.0f},
    m_camera_y{0.0f},
    m_camera_z{0.5f},
    m_camera_pitch{0.0f},
    m_camera_yaw{0.0f},
    m_camera_roll{0.0f}
    {
        setupDummyData();
        setFocus();
    }

void
rosy_gl_widget::maybeDrawNormals() const {
    if( !m_renderNormals) {
        return;
    }
    for( unsigned int i=0; i<m_positions.size() / 3; ++i) {
        glColor4f(m_normalColour.redF(),
                  m_normalColour.greenF(),
                  m_normalColour.blueF(),
                  m_normalColour.alphaF());
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glBegin(GL_LINES);
            glVertex3f( m_positions.at(i*3 + 0),
                        m_positions.at(i*3 + 1),
                        m_positions.at(i*3 + 2));
            glVertex3f( m_positions.at(i*3 + 0) + m_normals.at(i*3 + 0),
                        m_positions.at(i*3 + 1) + m_normals.at(i*3 + 1),
                        m_positions.at(i*3 + 2) + m_normals.at(i*3 + 2));
        glEnd();
    }
}

void
rosy_gl_widget::maybeDrawMainTangents() const {
    if( !m_renderMainTangents) {
        return ;
    }
    glColor4f(m_mainTangentColour.redF(),
              m_mainTangentColour.greenF(),
              m_mainTangentColour.blueF(),
              m_mainTangentColour.alphaF());
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);


    for( unsigned int i=0; i<m_positions.size() / 3; ++i) {
        glBegin(GL_LINES);
            glVertex3f( m_positions.at(i*3 + 0),
                        m_positions.at(i*3 + 1),
                        m_positions.at(i*3 + 2));
            glVertex3f( m_positions.at(i*3 + 0) + m_tangents.at(i*3 + 0),
                        m_positions.at(i*3 + 1) + m_tangents.at(i*3 + 1),
                        m_positions.at(i*3 + 2) + m_tangents.at(i*3 + 2));
        glEnd();
    }
}

void
rosy_gl_widget::maybeDrawOtherTangents() const {
    if( !m_renderOtherTangents) {
        return;
    }
    glColor4f(m_otherTangentsColour.redF(),
              m_otherTangentsColour.greenF(),
              m_otherTangentsColour.blueF(),
              m_otherTangentsColour.alphaF());
    glColor4f(0.0f, 1.0f, 1.0f, 1.0f);

    for( unsigned int i=0; i<m_positions.size() / 3; ++i) {
        // Get perpendicular tangent by computing cross(norm,tan)
        const auto normX = m_normals.at(i*3 + 0);
        const auto normY = m_normals.at(i*3 + 1);
        const auto normZ = m_normals.at(i*3 + 2);
        const auto tanX = m_tangents.at(i*3 + 0);
        const auto tanY = m_tangents.at(i*3 + 1);
        const auto tanZ = m_tangents.at(i*3 + 2);

        const float norm1 = distance_from_point_to_point(0,0,0,normX, normY, normZ);
        const float norm2 = distance_from_point_to_point(0,0,0,tanX, tanY, tanZ);
        const auto unitNormX = normX / norm1;
        const auto unitNormY = normY / norm1;
        const auto unitNormZ = normZ / norm1;
        const auto unitTanX = tanX / norm2;
        const auto unitTanY = tanY / norm2;
        const auto unitTanZ = tanZ / norm2;

        auto crossTanX = (unitNormY * unitTanZ - unitNormZ * unitTanY) * norm2; //bn -cm
        auto crossTanY = (unitNormZ * unitTanX - unitNormX * unitTanZ) * norm2; //bn -cm
        auto crossTanZ = (unitNormX * unitTanY - unitNormY * unitTanX) * norm2; //bn -cm



        glBegin(GL_LINES);
            glVertex3f( m_positions.at(i*3 + 0) - crossTanX,
                        m_positions.at(i*3 + 1) - crossTanY,
                        m_positions.at(i*3 + 2) - crossTanZ);
            glVertex3f( m_positions.at(i*3 + 0) + crossTanX,
                        m_positions.at(i*3 + 1) + crossTanY,
                        m_positions.at(i*3 + 2) + crossTanZ);
            glVertex3f( m_positions.at(i*3 + 0) - tanX,
                        m_positions.at(i*3 + 1) - tanY,
                        m_positions.at(i*3 + 2) - tanZ);
            glVertex3f( m_positions.at(i*3 + 0),
                        m_positions.at(i*3 + 1),
                        m_positions.at(i*3 + 2));
        glEnd();
    }
}


void
rosy_gl_widget::clear() {
    glClearColor(0.2f, 0.0f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT );
}

void
rosy_gl_widget::setModelViewMatrix() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(m_camera_pitch, 1, 0, 0);
    glRotatef(m_camera_yaw, 0, 1, 0);
    glRotatef(m_camera_roll, 0, 0, 1);
    glTranslatef(-m_camera_x, -m_camera_y, -m_camera_z);
}

void
rosy_gl_widget::setProjectionMatrix() {
        glMatrixMode(GL_PROJECTION);
        double tangent = tan(m_fov * DEG2RAD);   // tangent of half fovY
        double height = m_front * tangent;       // half height of near plane
        double width = height * m_aspect_ratio;
        glFrustum(-width, width, -height, height, m_front, m_back);
}

void
rosy_gl_widget::paintGL() {
    clear();

    glPushMatrix();

    setModelViewMatrix();

    setProjectionMatrix();

    maybeDrawNormals();

    maybeDrawMainTangents();

    maybeDrawOtherTangents();

    glPopMatrix();

    m_isDirty = false;
}

void
rosy_gl_widget::setRoSyData(const std::vector<float>& positions,
                 const std::vector<float>& normals,
                 const std::vector<float>& tangents ) {
    m_positions.clear();
    m_tangents.clear();
    m_normals.clear();

    m_positions.insert(m_positions.begin(), positions.begin(), positions.end());
    m_tangents.insert(m_tangents.begin(), tangents.begin(), tangents.end());
    m_normals.insert(m_normals.begin(), normals.begin(), normals.end());

    paintGL();
}

void
rosy_gl_widget::setupDummyData() {
    m_positions.push_back(-0.5f);
    m_positions.push_back(-0.5f);
    m_positions.push_back(-0.5f);
    m_positions.push_back( 0.5f);
    m_positions.push_back( 0.5f);
    m_positions.push_back( 0.5f);

    m_normals.push_back(0.0f);
    m_normals.push_back(0.1f);
    m_normals.push_back(0.0f);
    m_normals.push_back(0.0f);
    m_normals.push_back(0.1f);
    m_normals.push_back(0.0f);

    m_tangents.push_back(0.1f);
    m_tangents.push_back(0.0f);
    m_tangents.push_back(0.0f);
    m_tangents.push_back(0.0707f);
    m_tangents.push_back(0.0f);
    m_tangents.push_back(0.0707f);
}

void rosy_gl_widget::keyPressEvent(QKeyEvent *event) {
    float deltaPos = 0.1f;
    float deltaAngle = 1.0f;
    bool shiftDown = (event->modifiers() & Qt::KeyboardModifier::ShiftModifier) == Qt::KeyboardModifier::ShiftModifier;
    switch(event->key()) {
        case Qt::Key_Left:
            if(shiftDown){
                m_camera_roll -= deltaAngle;
                std::cout << "Roll left" << std::endl;
            }
            else {
                m_camera_x -= deltaPos;
                std::cout << "Left" << std::endl;
            }
            m_isDirty = true;
            break;

        case Qt::Key_Right:
            if(shiftDown){
                m_camera_roll += deltaAngle;
                std::cout << "Roll right" << std::endl;
            }
            else {
                m_camera_x += deltaPos;
                std::cout << "Right" << std::endl;
            }
            m_isDirty = true;
            break;

        case Qt::Key_Up:
            if( shiftDown){
                m_camera_pitch += deltaAngle;
                std::cout << "Pitch Forward" << std::endl;
            }
            else {
                m_camera_y += deltaPos;
                std::cout << "Up" << std::endl;
            }
            m_isDirty = true;
            break;

        case Qt::Key_Down:
            if( shiftDown){
                m_camera_pitch -= deltaAngle;
                std::cout << "Pitch Backwards" << std::endl;
            }
            else {
                m_camera_y -= deltaPos;
                std::cout << "Down" << std::endl;
            }
            m_isDirty = true;
            break;

        case Qt::Key_Equal:
            if( shiftDown){
                m_camera_yaw += deltaAngle;
                std::cout << "Yaw right" << std::endl;
            }
            else {
                m_camera_z += deltaPos;
                std::cout << "In" << std::endl;
            }
            m_isDirty = true;
            break;

        case Qt::Key_Minus:
            if( shiftDown){
                m_camera_yaw -= deltaAngle;
                std::cout << "Yawleft" << std::endl;
            }
            else {
                m_camera_z -= deltaPos;
                std::cout << "Out" << std::endl;
            }
            m_isDirty = true;
            break;

        default:
            std::cout << "Key pressed " << event->key() << std::endl;
    }
    if( m_isDirty) {
        update();
        m_isDirty = false;
    }
}
