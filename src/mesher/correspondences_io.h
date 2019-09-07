/**
 * Load and save correspondence data to file.
 * Created by Dave Durbin on 2019-07-06.
 */
#pragma once

#include <string>
#include <vector>
#include "mesher_args.h"

#include "types.h"

void
load_correspondences_from_file(const std::string &file_name,
                               std::vector<std::vector<PixelInFrame>> &correspondences);

void
save_correspondences_to_file(const std::string &file_name,
                             const std::vector<std::vector<PixelInFrame>> &correspondences);
