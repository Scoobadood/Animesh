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
                   const std::vector<float> &path,
                   float scale_factor);

  void renderNormals(bool shouldRender);

  void renderMainTangents(bool shouldRender);

  void renderOtherTangents(bool shouldRender);

  void renderErrorColours(bool shouldRender);

  void renderPath(bool shouldRender);

  void renderSplats(bool shouldRender);

  void set_norm_tan_length(float l);

 protected:
  void do_paint() override;

  void initializeGL() override;

  void click_at(unsigned int x, unsigned int y) override;

 private:
  std::vector<float> m_positions;
  std::vector<float> m_tangents;
  std::vector<float> m_normals;
  std::vector<float> m_colours;
  std::vector<float> m_path;
  float m_normalScaleFactor;
  bool m_renderNormals;
  bool m_renderPath;
  bool m_renderSplats;
  bool m_renderMainTangents;
  bool m_renderOtherTangents;
  bool m_renderErrorColours;
  QColor m_normalColour;
  QColor m_pathColour;
  QColor m_splatColour;
  QColor m_mainTangentColour;
  QColor m_otherTangentsColour;

  void drawPositions() const;

  void maybeDrawNormals() const;

  void maybeDrawSplats() const;

  void maybeDrawPath() const;

  void maybeDrawMainTangents() const;

  void maybeDrawOtherTangents() const;
};
