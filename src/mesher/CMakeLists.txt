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
set(Cpd_DIR "/usr/local/lib/cmake/cpd")
find_package(Cpd REQUIRED)

# Need spdlog
set(Fgt_DIR "${PROJECT_SOURCE_DIR}/third_party/build/fgt/lib/cmake/fgt")
set(SPDLOG_ROOT "/usr/local/include/spdlog")
find_package(spdlog REQUIRED)


# Need OMP
find_package(OpenMP )
if(NOT TARGET OpenMP::OpenMP_CXX)
    add_library(OpenMP_TARGET INTERFACE)
    add_library(OpenMP::OpenMP_CXX ALIAS OpenMP_TARGET)
    target_compile_options(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS})
    find_package(Threads REQUIRED)
    target_link_libraries(OpenMP_TARGET INTERFACE Threads::Threads)
    target_link_libraries(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS})
endif()

# Building the executable
add_executable(animesh
        depth_map_io.h depth_map_io.cpp
        correspondences_io.h correspondences_io.cpp
        io_utils.h io_utils.cpp
        optimise.h optimise.cpp
        surfel_compute.h surfel_compute.cpp
        surfel_io.h surfel_io.cpp
        utilities.h utilities.cpp
        hierarchical_mesher.cpp)

target_link_libraries(animesh
        Properties
        DepthMap
        Camera
        RoSy
        Cpd::Library-C++
#        OpenMP::OpenMP_CXX
        )

target_compile_features(animesh
        PUBLIC
        cxx_std_11)


# Unit Tests

# Include specific tests
# include_directories("${GTEST_INCLUDE_DIRS}")

file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable(
        testUtilities
        surfel_compute.cpp surfel_compute.h
        ${TEST_SOURCES})

target_link_libraries(testUtilities
        DepthMap
        Properties
        gtest gmock)

#add_test( NAME ZeroLengthVectorsShouldThrow COMMAND testVectorAngles --gtest_filter=ZeroLengthVectorsShouldThrow)

