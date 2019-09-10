cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

# Build type flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

# Need PCL
find_package(PCL 1.8 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Need CPD
set(Cpd_DIR "${PROJECT_SOURCE_DIR}/third_party/build/cpd/lib/cmake/cpd")
find_package(Cpd REQUIRED)

# Building the executable
#add_executable(mesher
#        mesher.cpp
#        smooth.cpp smooth.h
#        surfel_compute.cpp surfel_compute.h
#        mesher_args.cpp mesher_args.h
#        surfel_io.cpp surfel_io.h
#        correspondences_compute.cpp correspondences_compute.h
#        correspondences_io.cpp correspondences_io.h
#        io_utils.cpp io_utils.h
#        types.h surfel_hierarchy.cpp surfel_hierarchy.h depth_map_io.h depth_map_io.cpp)


# Building the executable
add_executable(animesh
        depth_map_io.h depth_map_io.cpp
        correspondences_compute.h correspondences_compute.cpp
        hierarchical_mesher.cpp)

target_link_libraries(animesh
        Properties
        DepthMap
        Camera
        Cpd::Library-C++
        )

#target_link_libraries(mesher
#        DepthMap
#        FileUtils
#        Graph
#        RoSy
#        Camera
#        ${PCL_LIBRARIES})

target_compile_features(animesh
        PUBLIC
        cxx_std_11)

#target_compile_features(mesher
#        PUBLIC
#        cxx_std_11)
