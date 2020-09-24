add_subdirectory(data_gen)

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

# Depth map renderer
add_executable(
		dmr
		dmr.cpp
		glutils.cpp
		mesh.cpp
		model.cpp
)
target_link_libraries(
		dmr
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