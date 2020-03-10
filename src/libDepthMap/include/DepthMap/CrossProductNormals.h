//
// Created by Dave Durbin on 9/3/20.
//

#ifndef ANIMESH_CROSSPRODUCTNORMALS_H
#define ANIMESH_CROSSPRODUCTNORMALS_H

#import <DepthMap/DepthMap.h>
#import <DepthMap/Normals.h>
#import <Camera/Camera.h>
#import <vector>


/**
 * Compute normals that have all adjacent data set in the depth map,
 * that is, those normals which can be computed fully from depth data alone.
 * // TODO: An alternative approach is here: https://cromwell-intl.com/3d/normals.html
 * // Currently using this: https://stackoverflow.com/questions/34644101/calculate-surface-normals-from-depth-image-using-neighboring-pixels-cross-produc
 *
 *
 */
std::vector<std::vector<NormalWithType>>
compute_natural_normals(const DepthMap * const depth_map, const Camera &camera);

void
compute_derived_normals(const DepthMap * const depth_map, std::vector<std::vector<NormalWithType>>& normals);


#endif //ANIMESH_CROSSPRODUCTNORMALS_H
