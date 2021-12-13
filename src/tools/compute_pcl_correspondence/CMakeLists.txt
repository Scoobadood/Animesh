# OBJ File Converter
add_executable(
		compute_correspondences
		main.cpp
)

target_link_libraries(
		compute_correspondences
		Correspondence
		Properties
		Surfel
		Tool
		Utilities
)

target_compile_features(
		compute_correspondences
		PUBLIC
		cxx_std_11
)