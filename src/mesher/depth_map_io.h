//
// Created by Dave Durbin on 2019-08-21.
//

#ifndef ANIMESH_DEPTH_MAP_IO_H
#define ANIMESH_DEPTH_MAP_IO_H

#include <vector>
#include <DepthMap/DepthMap.h>
#include "mesher_args.h"

std::vector<DepthMap>
load_depth_maps(const std::string& source_directory, float ts, float tl);

void
save_depth_map_as_pgm(const std::string& file_name, const DepthMap& depth_map);

void
save_normals_as_pgm(const std::string& file_name, const DepthMap& depth_map);

#endif //ANIMESH_DEPTH_MAP_IO_H
