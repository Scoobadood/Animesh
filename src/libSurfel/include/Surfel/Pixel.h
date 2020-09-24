//
// Created by Dave Durbin on 19/5/20.
//

#ifndef ANIMESH_PIXEL_H
#define ANIMESH_PIXEL_H

struct Pixel {
    Pixel(unsigned int x, unsigned int y);
    bool operator<(const Pixel &other) const;

    unsigned int x;
    unsigned int y;
};

inline bool operator==(const Pixel &lhs, const Pixel &rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}


#endif //ANIMESH_PIXEL_H
