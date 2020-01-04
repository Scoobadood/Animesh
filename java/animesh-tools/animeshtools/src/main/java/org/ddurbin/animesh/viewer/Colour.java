package org.ddurbin.animesh.viewer;

/**
 * A Colour as RGBA in floats
 */
class Colour {
    float red;
    float green;
    float blue;
    float alpha;
    Colour(float red, float green, float blue ) {
        this.red = red;
        this.green = green;
        this.blue = blue;
        this.alpha = 1.0f;
    }
}
