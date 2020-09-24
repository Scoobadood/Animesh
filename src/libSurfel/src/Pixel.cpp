//
// Created by Dave Durbin on 19/5/20.
//

#include "../include/Surfel/Pixel.h"

Pixel::Pixel(unsigned int x, unsigned int y) : x{x}, y{y} {}

bool Pixel::operator<(const Pixel &other) const {
    if (y != other.y)
        return y < other.y;

    return x < other.x;
}

