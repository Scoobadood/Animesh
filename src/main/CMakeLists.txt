# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "AnimeshConfig.h.in"
  "../include/AnimeshConfig.h"
  )

# Add the binary tree to the search path for inlcude files
file(GLOB SOURCES "*.cpp")

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Need vtk
find_package(VTK REQUIRED NO_MODULE) 
include_directories(${VTK_USE_FILE})
include(${VTK_USE_FILE})

# Building the executable
add_executable(Animesh 
	${SOURCES})


target_link_libraries(Animesh 
	Args
	Element
	Field
	FileUtils
	Graph
	RoSy
	${VTK_LIBRARIES} )

target_compile_features(Animesh 
	PUBLIC 
		cxx_std_11)
