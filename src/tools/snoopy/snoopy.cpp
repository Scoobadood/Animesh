//
// Created by Dave Durbin on 6/12/21.
//

#include "snoopy.h"

#include <DepthMap/DepthMapIO.h>
#include <DepthMap/DepthMap.h>

int main( int argc, const char * argv[]) {

    // Load snoopy images as depth maps
  DepthMap dm{"/Users/dave/Animesh/data/Snoopy/depth_000180.png"};
  dm.cull_unreliable_depths();
}