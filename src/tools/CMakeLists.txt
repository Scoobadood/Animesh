cmake_minimum_required (VERSION 3.8 FATAL_ERROR)


add_subdirectory( data_gen )

# GLFW
find_package(glfw3 3.2 REQUIRED)

# GLM
set(CMAKE_PREFIX_PATH, "/usr/local/include/glm:${CMAKE_PREFIX_PATH}")
find_package(glm REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")

# OpenGL
IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   INCLUDE_DIRECTORIES(${GLM_INCLUDE_DIR})
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
   MARK_AS_ADVANCED (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY)
   SET(EXTRA_LIBS ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
ENDIF (APPLE)
add_definitions(-DGL_SILENCE_DEPRECATION)


# ASSIMP
find_package(assimp REQUIRED)
if (assimp_FOUND)
    set(ASSIMP_LIBRARY "assimp")
    add_library(${ASSIMP_LIBRARY} SHARED IMPORTED)
    set_target_properties(${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.dylib")
endif(assimp_FOUND)

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
target_link_libraries(dmr
		glm)


# Need FGT
set(Fgt_DIR "${PROJECT_SOURCE_DIR}/third_party/build/fgt/lib/cmake/fgt")
find_package(Fgt REQUIRED)

# Need CPD
set(Cpd_DIR "${PROJECT_SOURCE_DIR}/third_party/build/cpd/lib/cmake/cpd")
find_package(Cpd REQUIRED)

# Correspondence pre-compute
add_executable(corr_util
		corr_util.cpp
		../mesher/io_utils.h ../mesher/io_utils.cpp
		../mesher/correspondences_compute.cpp ../mesher/correspondences_compute.h
		../mesher/correspondences_io.cpp ../mesher/correspondences_io.h
		../mesher/depth_map_io.cpp ../mesher/depth_map_io.h
		../mesher/types.h
		)


target_link_libraries(corr_util
		DepthMap
		Properties
		Camera
		Cpd::Library-C++)

add_executable(dm_to_point_cloud
		dm_to_point_cloud.cpp
		../mesher/io_utils.h ../mesher/io_utils.cpp
		../mesher/correspondences_compute.cpp ../mesher/correspondences_compute.h
		../mesher/correspondences_io.cpp ../mesher/correspondences_io.h
		../mesher/depth_map_io.cpp ../mesher/depth_map_io.h
		../mesher/types.h
		)

target_link_libraries(dm_to_point_cloud
		DepthMap
		Properties
		Camera
		Cpd::Library-C++)

target_link_libraries(dmr ${EXTRA_LIBS} glfw ${ASSIMP_LIBRARY})