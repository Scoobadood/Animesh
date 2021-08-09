//
// Created by Dave Durbin on 11/3/20.
//

#ifndef ANIMESH_PLANEFITTINGNORMALS_H
#define ANIMESH_PLANEFITTINGNORMALS_H

#include <vector>
#include <DepthMap/Normals.h>
#include <DepthMap/DepthMap.h>
#include <Camera/Camera.h>

/**
 * Compute normal using estinate of normal to plane tangent to surface
 * using neighbours in depth map
 */
std::vector<std::vector<NormalWithType>>
compute_normals_from_neighbours(DepthMap* depth_map, const Camera& camera);

#endif //ANIMESH_PLANEFITTINGNORMALS_H
