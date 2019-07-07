//
// Created by Dave Durbin on 2019-07-06.
//

#ifndef ANIMESH_TYPES_H
#define ANIMESH_TYPES_H

#include <Eigen/Core>

struct PixelInFrame {
    unsigned int x;
    unsigned int y;
    unsigned int frame;

    PixelInFrame(unsigned int x, unsigned int y, unsigned int frame) :x{x}, y{y}, frame{frame}{};

    bool operator< (const PixelInFrame &other) const {
        if( frame != other.frame)
            return frame < other.frame;

        if( y != other.y)
            return y < other.y;

        return x < other.x;
    }
};

struct FrameData {
    PixelInFrame	pixel_in_frame; 	// x, y, frame
    float 			depth;				// Depth of range scan at this pixel.
    Eigen::Matrix3f	transform{};			// Computed
    Eigen::Vector3f normal{};				// Normal at pixel in frame
    FrameData(const PixelInFrame& pif, float depth, const Eigen::Matrix3f& tran, const Eigen::Vector3f& norm) : pixel_in_frame{pif}, depth{depth}, transform{tran}, normal{norm}
    {}

    FrameData() : pixel_in_frame{0, 0, 0}, depth{0}, transform{Eigen::Matrix3f::Identity()}, normal{Eigen::Vector3f::Zero()}
    {}
};

struct Surfel {
    std::size_t 			id;
    std::vector<FrameData>	frame_data;
    std::vector<size_t>		neighbouring_surfels;
    Eigen::Vector3f 		tangent;
};

#endif //ANIMESH_TYPES_H
