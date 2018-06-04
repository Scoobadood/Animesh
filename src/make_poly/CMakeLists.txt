# Add the binary tree to the search path for inlcude files
file(GLOB SOURCES "*.cpp")

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# PCL
find_package(PCL 1.3 REQUIRED)
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

# Building the executable
add_executable(MakePoly ${SOURCES})

target_link_libraries(MakePoly 
	Args
	Field)


target_compile_features(MakePoly PUBLIC cxx_std_11)
