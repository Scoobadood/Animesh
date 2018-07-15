cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

# Add the binary tree to the search path for inlcude files
file(GLOB SOURCES "*.cpp")

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Need vtk
find_package(VTK REQUIRED NO_MODULE) 
include_directories(${VTK_USE_FILE})
include(${VTK_USE_FILE})


# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)


# Building the executable
add_executable(QtAnimesh 
	AnimeshMainWindow.ui
   	${SOURCES})

target_link_libraries(QtAnimesh 
	Args
	Field
	FileUtils
	Graph
	RoSy
	Qt5::Widgets
	${VTK_LIBRARIES} )

target_compile_features(QtAnimesh 
	PUBLIC 
		cxx_std_11)
