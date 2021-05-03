
# ASSIMP
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/third_party/lib/cmake/assimp-5.0")
find_package(assimp REQUIRED)
if (assimp_FOUND)
	set(ASSIMP_LIBRARY "assimp")
	set(ASSIMP_LIBRARY_DIRS ${ASSIMP_ROOT_DIR}/lib)
	add_library(${ASSIMP_LIBRARY} SHARED IMPORTED)
	set_target_properties(${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.dylib")
endif (assimp_FOUND)

add_subdirectory(data_gen)
add_subdirectory(glm)


list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/third_party/lib/cmake/cpd")
find_package(Cpd REQUIRED)

# OBJ File Converter
add_executable(
		obj_conv
		obj_converter.cpp
)

target_link_libraries(
		obj_conv
		FileUtils
		GeomFileUtils
)

target_compile_features(
		obj_conv
		PUBLIC
		cxx_std_11
)

# PLanar surfel file generator
add_executable(
		gen_surfels
		gen_surfel_graph.cpp
)
target_link_libraries(
		gen_surfels
		Surfel
)

# Depth map renderer
add_executable(
		dmr
		dmr.cpp
		glutils.cpp
		mesh.cpp
		model.cpp
)
target_include_directories(
		dmr PUBLIC glm
		${ASSIMP_INCLUDE_DIRS}
)

target_link_libraries(
		dmr glm::glm
		${ASSIMP_LIBRARY}
)

add_executable(
		dm_to_point_cloud dm_to_point_cloud.cpp
)

target_link_libraries(
		dm_to_point_cloud
		Camera
		Correspondence
		DepthMap
		GeomFileUtils
		Properties
		Surfel
		Utilities
		Cpd::Library-C++)

target_link_libraries(dmr ${EXTRA_LIBS} glfw ${ASSIMP_LIBRARY})

add_executable(
		gen_unsmoothed_rosy_graph
		gen_unsmoothed_rosy_graph.cpp
)

target_include_directories(
		gen_unsmoothed_rosy_graph
		PUBLIC
		../third_party/include
)

target_link_libraries(
		gen_unsmoothed_rosy_graph
		Surfel
)