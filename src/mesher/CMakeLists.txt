cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

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
	smooth.cpp
	surfel.cpp
	args.cpp
   	)

target_link_libraries(mesher
	DepthMap
	FileUtils
	Graph
	RoSy
	Camera
	${PCL_LIBRARIES})

target_compile_features(mesher
	PUBLIC
		cxx_std_11)
