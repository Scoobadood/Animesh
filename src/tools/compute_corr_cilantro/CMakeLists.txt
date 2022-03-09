# OBJ File Converter

find_package(cilantro)

add_executable(
		compute_correspondences_cil
		main.cpp
)

target_link_libraries(
		compute_correspondences_cil
		Correspondence
		Properties
		Surfel
		Tool
		Utilities
		cilantro
)

target_compile_features(
		compute_correspondences_cil
		PUBLIC
		cxx_std_11
)