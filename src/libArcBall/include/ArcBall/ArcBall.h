//
// Created by Dave Durbin (Old) on 10/4/21.
//

#pragma once

#include <QVector3D>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QObject>
#include <QMatrix4x4>

class ArcBall : public QObject {
public:
    ArcBall();

    void rotate(float dTheta, float dPhi);

    void zoom(float distance);

    void pan(float dx, float dy);

    void get_model_view_matrix(float mat[16]);

private:
    const float ROTATE_FACTOR = 300.0f;
    const float PAN_FACTOR = 0.01f;
    const float ZOOM_FACTOR = 0.005f;

    float m_theta;
    float m_phi;
    float m_radius;
    float m_up;
    bool m_modelViewMatrixIsDirty;
    QPoint m_lastPixelPosition;
    QVector3D m_target;

    QVector3D getCameraPosition() const;

    QVector3D toCartesian() const;

    QMatrix4x4 m_model_view_matrix;

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

    void mouseMoveEvent(QMouseEvent *e);

    void mousePressEvent(QMouseEvent *e);

    void mouseReleaseEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);

    void wheelEvent(QWheelEvent *e);
};