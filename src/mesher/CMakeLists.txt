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


# Building the executable
add_executable(mesher
        mesher.cpp
        smooth.cpp smooth.h
        surfel_compute.cpp surfel_compute.h
        mesher_args.cpp mesher_args.h
        surfel_io.cpp surfel_io.h
        correspondences_compute.cpp correspondences_compute.h
        correspondences_args.cpp correspondences_args.h
        correspondences_io.cpp correspondences_io.h
        io_utils.cpp io_utils.h
        types.h surfel_hierarchy.cpp surfel_hierarchy.h depth_map_io.h depth_map_io.cpp)


# Building the executable
add_executable(animesh
        hierarchical_mesher.cpp)

# Building the executable
add_executable(correspondences
        correspondences.cpp
        correspondences_compute.cpp correspondences_compute.h
        correspondences_args.cpp correspondences_args.h
        correspondences_io.cpp  correspondences_io.h
        types.h
        io_utils.cpp io_utils.h)

target_link_libraries(animesh
        Properties
        )

target_link_libraries(mesher
        DepthMap
        FileUtils
        Graph
        RoSy
        Camera
        ${PCL_LIBRARIES})

target_link_libraries(correspondences
        FileUtils
        )

target_compile_features(animesh
        PUBLIC
        cxx_std_11)

target_compile_features(mesher
        PUBLIC
        cxx_std_11)

target_compile_features(correspondences
        PUBLIC
        cxx_std_11)
