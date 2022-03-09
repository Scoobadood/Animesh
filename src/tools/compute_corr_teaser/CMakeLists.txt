# OBJ File Converter

list(APPEND CMAKE_PREFIX_PATH "/Users/dave/bin/lib/cmake/teaserpp")
find_package(teaserpp REQUIRED)

list(APPEND CMAKE_PREFIX_PATH "/Users/dave/bin/lib/cmake/tinyply")
find_package(tinyply REQUIRED)

add_executable(
		compute_correspondences_tsr
		main.cpp
)

message("tinyply_PREFIX is " ${tinyply_PREFIX})

find_library (
		tinyply_library
		tinyply
		PATHS ${tinyply_PREFIX}/lib
		REQUIRED
		NO_SYSTEM_ENVIRONMENT_PATH
)

message("tinyply_library is " ${tinyply_library})

target_link_libraries(
		compute_correspondences_tsr
		PUBLIC
		Correspondence
		Properties
		Surfel
		Tool
		Utilities
		teaserpp::teaser_registration
		teaserpp::teaser_io
		${tinyply_library}
)

target_compile_features(
		compute_correspondences_tsr
		PUBLIC
		cxx_std_11
)