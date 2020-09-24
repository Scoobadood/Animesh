//
// Created by Dave Durbin on 19/5/20.
//

#ifndef ANIMESH_PIXELINFRAME_H
#define ANIMESH_PIXELINFRAME_H

#include <iostream>
#include "Pixel.h"

struct PixelInFrame {
    PixelInFrame(unsigned int x, unsigned int y, unsigned int frame);
    PixelInFrame(Pixel pixel, unsigned int frame);
    bool operator<(const PixelInFrame &other) const;

    Pixel pixel;
    unsigned int frame;
};

inline bool operator==(const PixelInFrame &lhs, const PixelInFrame &rhs) {
    return (lhs.pixel == rhs.pixel) && (lhs.frame == rhs.frame);
}

inline std::ostream &operator<<(std::ostream &os, const PixelInFrame &pif) {
    using namespace std;
    os << "{ f: " << pif.frame << " x: " << pif.pixel.x << " y: " << pif.pixel.y << " }";
    return os;
}


#endif //ANIMESH_PIXELINFRAME_H
