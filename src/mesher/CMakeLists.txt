cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

# Build type flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

file(GLOB SOURCES "*.cpp")

# Need VCG
find_package(VCG REQUIRED)
include_directories(${VCG_INCLUDE_DIR})


# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})


# Building the executable
add_executable(mesher
   	${SOURCES})

target_link_libraries(mesher
	FileUtils
	${PCL_LIBRARIES})

target_compile_features(mesher
	PUBLIC
		cxx_std_11)
