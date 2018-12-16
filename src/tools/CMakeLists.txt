cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

# OBJ File Converter
add_executable(obj_conv
   	obj_converter.cpp)

target_link_libraries(obj_conv
	FileUtils)

target_compile_features(obj_conv
	PUBLIC
		cxx_std_11)

# Perf Tool
add_executable(perf_tool
   	perfTool.cpp)

target_link_libraries(perf_tool
  Args
	Field
	FileUtils
	Graph
	RoSy)

target_compile_features(perf_tool
	PUBLIC
		cxx_std_11)

# Depth map renderer
add_executable(dmr
   	dmr.cpp)


# GLFW
find_package(glfw3 3.2 REQUIRED)

# OpenGL
IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
   MARK_AS_ADVANCED (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY)
   SET(EXTRA_LIBS ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
ENDIF (APPLE)

# link_directories(/usr/X11/lib)
# SET(EXTRA_LIBS GL X11 GLU glut)
# include_directories(/usr/X11/include/)
target_link_libraries(dmr ${EXTRA_LIBS} glfw)

target_compile_features(dmr
	PUBLIC
		cxx_std_11)
