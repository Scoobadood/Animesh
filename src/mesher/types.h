//
// Created by Dave Durbin on 2019-07-06.
//
#pragma once

#ifndef ANIMESH_TYPES_H
#define ANIMESH_TYPES_H

#include <Eigen/Core>
#include <utility>
#include <map>

struct Pixel {
    unsigned int x;
    unsigned int y;

    Pixel(unsigned int x, unsigned int y) : x{x}, y{y} {
        // Empty
    };

    bool operator<(const Pixel &other) const {
        if (y != other.y)
            return y < other.y;

        return x < other.x;
    }
};

struct PixelInFrame {
    Pixel pixel;
    unsigned int frame;

    PixelInFrame(unsigned int x, unsigned int y, unsigned int frame) : pixel{x, y}, frame{frame} {};

    PixelInFrame(Pixel pixel, unsigned int frame) : pixel{pixel}, frame{frame} {};

    bool operator<(const PixelInFrame &other) const {
        if (frame != other.frame)
            return frame < other.frame;

        return pixel < other.pixel;
    }

};

inline bool operator==(const Pixel &lhs, const Pixel &rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

inline bool operator==(const PixelInFrame &lhs, const PixelInFrame &rhs) {
    return (lhs.pixel == rhs.pixel) && (lhs.frame == rhs.frame);
}

inline std::ostream &operator<<(std::ostream &os, const PixelInFrame &pif) {
    using namespace std;
    os << "{ f: " << pif.frame << " x: " << pif.pixel.x << " y: " << pif.pixel.y << " }";
    return os;
}

struct SurfelInFrame {
    std::string surfel_id;
    size_t frame_index;

    SurfelInFrame(std::string s, size_t f) : surfel_id{std::move(s)}, frame_index{f} {}

    bool operator<(const SurfelInFrame &other) const {
        if (frame_index != other.frame_index)
            return frame_index < other.frame_index;

        return surfel_id < other.surfel_id;
    }
};

struct NormalTangent {
    Eigen::Vector3f normal;
    Eigen::Vector3f tangent;

    NormalTangent(Eigen::Vector3f n, Eigen::Vector3f t) : normal{std::move(n)}, tangent{std::move(t)} {
    }
};

struct FrameData {
    PixelInFrame pixel_in_frame;    // x, y, frame
    float depth;                // Depth of range scan at this pixel.
    Eigen::Matrix3f transform{};            // Computed
    Eigen::Vector3f normal{};                // Normal at pixel in frame
    FrameData(const PixelInFrame &pif, float depth, Eigen::Matrix3f tran, Eigen::Vector3f norm) : pixel_in_frame{
            pif}, depth{depth}, transform{std::move(tran)}, normal{std::move(norm)} {}

    FrameData() : pixel_in_frame{0, 0, 0}, depth{0}, transform{Eigen::Matrix3f::Identity()},
                  normal{Eigen::Vector3f::Zero()} {}

    bool operator<(const FrameData &other) const {
        return pixel_in_frame.frame < other.pixel_in_frame.frame;
    }
};

struct Surfel {
    static std::map<std::string, std::reference_wrapper<Surfel>> surfel_by_id;

    std::string id;
    std::vector<FrameData> frame_data;
    std::vector<std::string> neighbouring_surfels;
    Eigen::Vector3f tangent;
    Surfel(const std::string& id, const std::vector<FrameData>& frames, const std::vector<std::string>& neighbours, const Eigen::Vector3f& tangent) :
    id{id}, tangent{tangent} {
        for( auto & fd : frames ) { frame_data.push_back(fd);}
        for( const auto & nb : neighbours ) { neighbouring_surfels.push_back(nb);}
    }
    Surfel(){}
};

#endif //ANIMESH_TYPES_H
