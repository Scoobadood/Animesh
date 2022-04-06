# Animesh 

Animesh is my PhD project designed to generate an animation ready mesh from point cloud fusion

## Prereqs
This code has been built on Ubuntu 16. It requires the Google C++ Test framework if you intend to build and run tests. Installation instructions are here:
https://www.eriksmistad.no/getting-started-with-google-test-on-ubuntu/

Eigen3 is required for vector math


## Build process
cd build
ccmake ..
cmake ..
make




# Release Notes
Implementing dumping of hierarchical graphs when we do multi-resolution smoothing because we're getting results that seem unlikely.
In particular, high level smoothing is not fixing conflicts in lower level graphs.

Possible issues are a problem with the MultiResolutionGraph (propagation or otherwise) or else with the order in which it's bult - if nodes are not randomised.

* Removed issue where multi-resolution graphs had self links