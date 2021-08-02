//
// Created by Dave Durbin (Old) on 2/8/21.
//

#ifndef ANIMESH_TOOLS_FIELD_VISUALISER_GL_TOOLS_H
#define ANIMESH_TOOLS_FIELD_VISUALISER_GL_TOOLS_H

/**
 * Compute a vector for the direction form the camera origin through a given pixel.
 * @param pixel_coord The pixel
 * @return The unit vector
 */
Eigen::Vector3f compute_ray_through_pixel(unsiged int x, untisgned int y);

#endif //ANIMESH_TOOLS_FIELD_VISUALISER_GL_TOOLS_H
