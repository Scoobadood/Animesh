//
// Created by Dave Durbin on 7/3/20.
//

#ifndef ANIMESH_PCLNORMALS_H
#define ANIMESH_PCLNORMALS_H

#include <vector>
#include <DepthMap/Normals.h>
#include <DepthMap/DepthMap.h>
#include <Camera/Camera.h>

/**
 * Compute normal using estinate of normal to plane tangent to surface
 */

std::vector<std::vector<NormalWithType>>
compute_normals_with_pcl(DepthMap* depth_map, const Camera& camera);

#endif //ANIMESH_PCLNORMALS_H
