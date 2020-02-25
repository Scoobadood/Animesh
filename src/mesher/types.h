//
// Created by Dave Durbin on 2019-07-06.
//
#pragma once

#ifndef ANIMESH_TYPES_H
#define ANIMESH_TYPES_H

#include <Eigen/Core>
struct Pixel {
    unsigned int x;
    unsigned int y;
    Pixel(unsigned int x, unsigned int y) : x{x}, y{y}{
        // Empty
    };
    bool operator< (const Pixel &other) const {
        if( y != other.y)
            return y < other.y;

        return x < other.x;
    }
};

struct PixelInFrame {
    Pixel pixel;
    unsigned int frame;

    PixelInFrame(unsigned int x, unsigned int y, unsigned int frame) : pixel{x, y}, frame{frame}{};
    PixelInFrame(Pixel pixel, unsigned int frame) : pixel{pixel}, frame{frame}{};

    bool operator< (const PixelInFrame &other) const {
        if( frame != other.frame)
            return frame < other.frame;

        return pixel < other.pixel;
    }

};

inline bool operator==(const Pixel& lhs, const Pixel& rhs){
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

inline bool operator==(const PixelInFrame& lhs, const PixelInFrame& rhs){
    return (lhs.pixel == rhs.pixel) && (lhs.frame == rhs.frame);
}

inline std::ostream &operator<<(std::ostream &os, const PixelInFrame &pif) {
    using namespace std;
    os << "{ f: " << pif.frame << " x: " << pif.pixel.x << " y: " << pif.pixel.y << " }";
    return os;
}

struct SurfelInFrame {
    size_t frame_index;
    size_t surfel_index;

    SurfelInFrame(size_t s, size_t f) {
        frame_index = f;
        surfel_index = s;
    }
    bool operator<(const SurfelInFrame& other) const {
        if( frame_index != other.frame_index)
            return frame_index < other.frame_index;

        return surfel_index < other.surfel_index;
    }
};

struct NormalTangent {
    Eigen::Vector3f normal;
    Eigen::Vector3f tangent;
    NormalTangent(Eigen::Vector3f n, Eigen::Vector3f t) : normal{n}, tangent{t} {
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

    bool operator<(const FrameData& other) const {
        return pixel_in_frame.frame < other.pixel_in_frame.frame;
    }
};

struct Surfel {
    std::size_t 			id;
    std::vector<FrameData>	frame_data;
    std::vector<size_t>		neighbouring_surfels;
    Eigen::Vector3f 		tangent;
};

#endif //ANIMESH_TYPES_H
