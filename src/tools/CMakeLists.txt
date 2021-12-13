
# ASSIMP
find_package(assimp REQUIRED)
if (assimp_FOUND)
	set(ASSIMP_LIBRARY "assimp")
	set(ASSIMP_LIBRARY_DIRS ${ASSIMP_ROOT_DIR}/lib)
	add_library(${ASSIMP_LIBRARY} SHARED IMPORTED paths_to_surfel_graph/paths_to_surfels.cpp paths_to_surfel_graph/paths_to_surfels.h snoopy/snoopy.cpp snoopy/snoopy.h)
	set_target_properties(${ASSIMP_LIBRARY} PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIBRARY_DIRS}/libassimp.dylib")
endif (assimp_FOUND)

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
		CommonUtilities
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

add_executable(
		gen_unsmoothed_rosy_graph
		gen_unsmoothed_rosy_graph.cpp
)

target_include_directories(
		gen_unsmoothed_rosy_graph
		PUBLIC
)

target_link_libraries(
		gen_unsmoothed_rosy_graph
		Surfel
		spdlog::spdlog
)

add_executable(
		surfel_bin_to_txt
		surfel_bin_to_txt.cpp
)
target_include_directories(
		surfel_bin_to_txt
		PUBLIC
		../third_party/include
)

target_link_libraries(
		surfel_bin_to_txt
		Surfel
		FileUtils
)

add_executable(
		surfel_scaler
		surfel_scalar.cpp
)
target_include_directories(
		surfel_scaler
		PUBLIC
		../third_party/include
)

target_link_libraries(
		surfel_scaler
		Surfel
		FileUtils
)

# Convert an OBJ file to a Surfel graph
add_executable(
		obj_to_graph
		obj_to_graph.h obj_to_graph.cpp
)
target_include_directories(
		obj_to_graph
		PUBLIC
)
target_link_libraries(
		obj_to_graph
		Surfel
		FileUtils
		GeomFileUtils
		spdlog::spdlog
)

add_subdirectory(compute_pcl_correspondence)
add_subdirectory(data_gen)
add_subdirectory(dmr)
add_subdirectory(dm_to_data)
add_subdirectory(dm_to_graph)
add_subdirectory(field_visualiser)
add_subdirectory(leggings_tool)
add_subdirectory(libTool)
add_subdirectory(multi-res-tool)
add_subdirectory(optimisers)
add_subdirectory(paths_to_surfel_graph)
add_subdirectory(quadulator)
#add_subdirectory(snoopy)
add_subdirectory(surfel_info)
