# Legging Tool
add_executable(
		leg_tool
		main.cpp
)

target_include_directories(
		leg_tool
		PRIVATE
)

target_link_libraries(
		leg_tool
		FileUtils
		GeomFileUtils
		Surfel
		spdlog::spdlog
)

target_compile_features(
		leg_tool
		PUBLIC
		cxx_std_11
)