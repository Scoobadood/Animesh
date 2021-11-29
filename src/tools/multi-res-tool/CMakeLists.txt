# Legging Tool
add_executable(
		multi_res_tool
		main.cpp
)

target_include_directories(
		multi_res_tool
		PRIVATE
)
target_link_libraries(
		multi_res_tool
		Surfel
		spdlog::spdlog
)

target_compile_features(
		multi_res_tool
		PUBLIC
		cxx_std_11
)