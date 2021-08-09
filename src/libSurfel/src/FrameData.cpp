//
// Created by Dave Durbin on 19/5/20.
//

#include "FrameData.h"

FrameData::FrameData(const PixelInFrame &pif,
                     float depth,
                     Eigen::Matrix3f tran,
                     Eigen::Vector3f norm,
                     Eigen::Vector3f pos) :
        pixel_in_frame{pif},
        depth{depth},
        transform{std::move(tran)},
        normal{std::move(norm)},
        position{std::move(pos)} {
}

FrameData::FrameData() :
        pixel_in_frame{0, 0, 0},
        depth{0.0f},
        transform{Eigen::Matrix3f::Identity()},
        normal{Eigen::Vector3f::Zero()},
        position{Eigen::Vector3f::Zero()} {
}

bool FrameData::operator<(const FrameData &other) const {
    return pixel_in_frame.frame < other.pixel_in_frame.frame;
}

