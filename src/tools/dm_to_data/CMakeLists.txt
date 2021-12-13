# OBJ File Converter
add_executable(
		dm_to_data
		main.cpp
)

target_link_libraries(
		dm_to_data
		DepthMap
		Properties
		Surfel
		Utilities
)

target_compile_features(
		dm_to_data
		PUBLIC
		cxx_std_11
)