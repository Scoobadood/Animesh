//
// Created by Dave Durbin on 19/5/20.
//

#ifndef ANIMESH_FRAMEDATA_H
#define ANIMESH_FRAMEDATA_H

#include <Eigen/Core>
#include "PixelInFrame.h"

struct FrameData {
    FrameData();

    FrameData(const PixelInFrame &pif,
              float depth,
              Eigen::Matrix3f tran,
              Eigen::Vector3f norm,
              Eigen::Vector3f pos
    );

    bool operator<(const FrameData &other) const;

    PixelInFrame pixel_in_frame;    // x, y, frame
    float depth;                    // Depth of range scan at this pixel.
    Eigen::Matrix3f transform;      // Computed
    Eigen::Vector3f normal;         // Normal at pixel in frame
    Eigen::Vector3f position;       // Position of the surfel in this frame
};

#endif //ANIMESH_FRAMEDATA_H
