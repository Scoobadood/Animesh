# OBJ File Converter
add_executable(
		paths_to_surfels
		paths_to_surfels.cpp
)

target_link_libraries(
		paths_to_surfels
		FileUtils
		CommonUtilities
		Correspondence
		spdlog::spdlog
)

target_compile_features(
		paths_to_surfels
		PUBLIC
		cxx_std_11
)