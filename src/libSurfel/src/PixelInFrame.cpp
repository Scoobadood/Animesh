//
// Created by Dave Durbin on 19/5/20.
//

#include "../include/Surfel/PixelInFrame.h"
#include "../include/Surfel/Pixel.h"

PixelInFrame::PixelInFrame(unsigned int x, unsigned int y, unsigned int frame) : pixel{x, y}, frame{frame} {};

PixelInFrame::PixelInFrame(Pixel pixel, unsigned int frame) : pixel{pixel}, frame{frame} {};

bool PixelInFrame::operator<(const PixelInFrame &other) const {
    if (frame != other.frame)
        return frame < other.frame;

    return pixel < other.pixel;
}
