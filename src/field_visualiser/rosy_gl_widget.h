#pragma once

#include <QColor>
#include <QVector3D>
#include <QMatrix4x4>
#include <Surfel/SurfelGraph.h>
#include <ArcBall/ArcBall.h>
#include <vector>
#include "field_gl_widget.h"

/**
 * @brief The rosy_gl_widget class.
 * Renders RoSy with trackball controls and allows selection of individual surfels.
 * Interface contract:
 * Can accept new data and redraw
 * Can handle mouse selection of a surfel and emits a selected event
 * Can locally manage orientation of the view.
 * Maintains state about what to render (normals, tangents, etc.)
 */
class rosy_gl_widget : public field_gl_widget {
Q_OBJECT
public:
    explicit rosy_gl_widget(QWidget *parent = nullptr, //
                            Qt::WindowFlags f = Qt::WindowFlags());

    void setRoSyData(const std::vector<float> &positions,
                     const std::vector<float> &normals,
                     const std::vector<float> &tangents,
                     const std::vector<float> &colours,
                     float scale_factor);

    void renderNormals(bool shouldRender);

    void renderMainTangents(bool shouldRender);

    void renderOtherTangents(bool shouldRender);

    void renderErrorColours(bool shouldRender);

protected:
    void do_paint() override;

    void initializeGL() override;

private:
    std::vector<float> m_positions;
    std::vector<float> m_tangents;
    std::vector<float> m_normals;
    std::vector<float> m_colours;
    float m_normalScaleFactor;
    bool m_renderNormals;
    bool m_renderMainTangents;
    bool m_renderOtherTangents;
    bool m_renderErrorColours;
    QColor m_normalColour;
    QColor m_mainTangentColour;
    QColor m_otherTangentsColour;

    void drawPositions() const;

    void maybeDrawNormals() const;

    void maybeDrawMainTangents() const;

    void maybeDrawOtherTangents() const;
};
