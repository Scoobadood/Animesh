#ifndef POSY_GL_WIDGET_H
#define POSY_GL_WIDGET_H

#include <QOpenGLWidget>
#include <Surfel/SurfelGraph.h>
#include <vector>

class posy_gl_widget : public QOpenGLWidget
{
    Q_OBJECT
public:
    posy_gl_widget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags()): QOpenGLWidget{parent, f}
    {
        positions.push_back(0.0f);
        positions.push_back(0.0f);
        positions.push_back(0.0f);
        positions.push_back(2.0f);
        positions.push_back(2.0f);
        positions.push_back(2.0f);

        normals.push_back(0.0f);normals.push_back(1.0f);normals.push_back(0.0f);normals.push_back(0.0f);normals.push_back(1.0f);normals.push_back(0.0f);
        tangents.push_back(1.0f);tangents.push_back(0.0f);tangents.push_back(0.0f);tangents.push_back(1.0f);tangents.push_back(0.0f);tangents.push_back(0.0f);
    };
    void setPoSyData(const SurfelGraph& graph);
protected:
    void paintGL();
private:
    std::vector<float> positions;
    std::vector<float>tangents;
    std::vector<float>normals;
    int frame = 0;
};

#endif // POSY_GL_WIDGET_H
