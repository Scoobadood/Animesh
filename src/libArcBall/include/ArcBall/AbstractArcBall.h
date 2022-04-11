//
// Created by Dave Durbin (Old) on 20/9/21.
//

#ifndef ANIMESH_LIBARCBALL_INCLUDE_ARCBALL_ABSTRACTARCBALL_H
#define ANIMESH_LIBARCBALL_INCLUDE_ARCBALL_ABSTRACTARCBALL_H

class AbstractArcBall : public QObject{
public:
  virtual void get_model_view_matrix(float mat[16]) = 0;
  virtual QVector3D get_camera_origin() const = 0;
};

#endif //ANIMESH_LIBARCBALL_INCLUDE_ARCBALL_ABSTRACTARCBALL_H
