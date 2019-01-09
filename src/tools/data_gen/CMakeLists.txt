cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

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
add_definitions(-DGL_SILENCE_DEPRECATION)


# OpenCL 
find_package(OpenCL REQUIRED)
IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(OpenCL_LIBRARY OpenCL )
   MARK_AS_ADVANCED (OpenCL_LIBRARY)
ENDIF (APPLE)
add_definitions(-DCL_SILENCE_DEPRECATION)


# ASSIMP
find_package(assimp REQUIRED)
if (assimp_FOUND)
    set(ASSIMP_LIBRARY "assimp")
    add_library(${ASSIMP_LIBRARY} SHARED IMPORTED)
    set_target_properties(${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.dylib")
endif(assimp_FOUND)

# Data Generator
add_executable(dat_gen
    dat_gen.cpp
      mesh_cl.cpp
      model_cl.cpp)

target_link_libraries(dat_gen 
  FileUtils
  glm 
  OpenCL::OpenCL 
  ${EXTRA_LIBS} 
  ${ASSIMP_LIBRARY})

