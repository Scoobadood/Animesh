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
   	dmr.cpp
    glutils.cpp
      mesh.cpp
      model.cpp)

# Data Generator
add_executable(dat_gen
    dat_gen.cpp
      mesh_cl.cpp
      model_cl.cpp)

# GLFW
find_package(glfw3 3.2 REQUIRED)

# GLM
find_package(glm REQUIRED)

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


# OpenCL 
find_package(OpenCL REQUIRED)
IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(OpenCL_LIBRARY OpenCL )
   MARK_AS_ADVANCED (OpenCL_LIBRARY)
ENDIF (APPLE)


# ASSIMP
find_package(assimp REQUIRED)
if (assimp_FOUND)
    set(ASSIMP_LIBRARY "assimp")
    add_library(${ASSIMP_LIBRARY} SHARED IMPORTED)
    set_target_properties(${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.dylib")
endif(assimp_FOUND)

target_link_libraries(dmr ${EXTRA_LIBS} glfw ${ASSIMP_LIBRARY})

target_link_libraries(dat_gen 
  FileUtils
  glm 
  OpenCL::OpenCL 
  ${EXTRA_LIBS} 
  ${ASSIMP_LIBRARY})

add_definitions(-DGL_SILENCE_DEPRECATION)
add_definitions(-DCL_SILENCE_DEPRECATION)