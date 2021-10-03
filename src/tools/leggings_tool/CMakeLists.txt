# Legging Tool
add_executable(
		leg_tool
		main.cpp
)

target_include_directories(
		leg_tool
		PRIVATE
		${PROJECT_SOURCE_DIR}/third_party/include/Ei
		${PROJECT_SOURCE_DIR}/third_party/include
)
target_link_libraries(
		leg_tool
		FileUtils
		GeomFileUtils
		Surfel
)

target_compile_features(
		leg_tool
		PUBLIC
		cxx_std_11
)