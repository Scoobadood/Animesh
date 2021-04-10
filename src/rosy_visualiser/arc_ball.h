//
// Created by Dave Durbin (Old) on 10/4/21.
//

#ifndef ANIMESH_ARC_BALL_H
#define ANIMESH_ARC_BALL_H

#include <QVector3D>
#include <QObject>

class arc_ball : public QObject {
public:
    arc_ball();

    void rotate(float dTheta, float dPhi);

    void zoom(float distance);

    void pan(float dx, float dy);

    void modelViewMatrix(float[16]);

    bool modelViewMatrixHasChanged() const {
        return m_modelViewMatrixIsDirty;
    }

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

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

    void mouseMoveEvent(QMouseEvent *e);

    void mousePressEvent(QMouseEvent *e);

    void mouseReleaseEvent(QMouseEvent *e);

    void keyPressEvent(QKeyEvent *e);

    void wheelEvent(QWheelEvent *e);
};

#endif //ANIMESH_ARC_BALL_H
