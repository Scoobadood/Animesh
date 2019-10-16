#include <ostream>
#include <string>
#include <Eigen/Core>

#pragma once

typedef struct Camera {
    float position[3];
    float view[3];
    float up[3];
    float resolution[2];
    float fov[2];
    float focalDistance;

    Camera(){}
    Camera(const float pos[3], const float v[3], const float u[3], const float res[2], const float fv[2], float f)
            : position{pos[0], pos[1], pos[2]},
              view{v[0], v[1], v[2]},
              up{u[0], u[1], u[2]},
              resolution{res[0], res[1]},
              fov{fv[0], fv[1]},
              focalDistance{f} {};
} Camera;

Camera loadCameraFromFile(const std::string &filename);

void decomposeCamera(const Camera &camera, Eigen::Matrix3f &K, Eigen::Matrix3f &R, Eigen::Vector3f &t);

std::ostream &operator<<(std::ostream &os, const Camera &camera);

Eigen::Vector3f backproject(const Camera &camera, int pixel_x, int pixel_y, float depth);

void
backproject(const Camera &camera, int pixel_x, int pixel_y, float depth, float *world_x, float *world_y,
            float *world_z);
