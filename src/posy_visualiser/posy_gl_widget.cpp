#include "posy_gl_widget.h"

#include "QOpenGLFunctions"
#include <iostream>
#include <Surfel/SurfelGraph.h>
#include <vector>

void
posy_gl_widget::paintGL() {
    glClearColor(0.4f, 0.4f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glFrustum(-1, 1, -1, 1, 0.5f, 10.0f);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    for( int i=0; i<positions.size() / 3; i++) {

    // Draw a set of axes
        glColor3f(1.0,0.0,0.0);
        glBegin(GL_LINES);
            glVertex3f( positions.at(i*3 + 0),
                        positions.at(i*3 + 1),
                        positions.at(i*3 + 2));
            glVertex3f( positions.at(i*3 + 0) + normals.at(i*3 + 0),
                        positions.at(i*3 + 1) + normals.at(i*3 + 1),
                        positions.at(i*3 + 2) + normals.at(i*3 + 2));
        glEnd();

        glColor3f(0.0,1.0,0.0);
        glBegin(GL_LINES);
        glVertex3f( positions.at(i*3 + 0),
                    positions.at(i*3 + 1),
                    positions.at(i*3 + 2));
        glVertex3f( positions.at(i*3 + 0) + tangents.at(i*3 + 0),
                    positions.at(i*3 + 1) + tangents.at(i*3 + 1),
                    positions.at(i*3 + 2) + tangents.at(i*3 + 2));
        glEnd();

        glUseProgram(0);
        glFlush();
    }

}

void posy_gl_widget::setPoSyData(const SurfelGraph& graph) {
    positions.clear();
    tangents.clear();
    normals.clear();
    for( const auto & node : graph.nodes()) {
        const auto & surfel = node->data();
        if( surfel->is_in_frame(frame)) {
            Eigen::Vector3f position;
            Eigen::Vector3f tangent;
            Eigen::Vector3f normal;
            surfel->get_position_tangent_normal_for_frame(frame, position, tangent, normal);
            positions.push_back(position.x());
            positions.push_back(position.y());
            positions.push_back(position.z());

            tangents.push_back(tangent.x());
            tangents.push_back(tangent.y());
            tangents.push_back(tangent.z());

            normals.push_back(normal.x());
            normals.push_back(normal.y());
            normals.push_back(normal.z());
        }
    }

}



