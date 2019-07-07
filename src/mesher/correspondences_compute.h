//
// Created by Dave Durbin on 2019-07-06.
//

#ifndef ANIMESH_CORRESPONDENCES_COMPUTE_H
#define ANIMESH_CORRESPONDENCES_COMPUTE_H

#include <string>
#include <vector>
#include "types.h"

/**
 * Compute the correspondences between pixels in cleaned depth maps
 * Returns a vector or correspondences where each correspondence
 * is a PixelInFrame
 */
void
compute_correspondences(const std::string &source_directory,
                        std::vector<std::vector<PixelInFrame>> &correspondences);

#endif //ANIMESH_CORRESPONDENCES_COMPUTE_H
