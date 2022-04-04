# OBJ File Converter
add_executable(
		obj_to_graph
		obj_to_graph.cpp
)

target_link_libraries(
		obj_to_graph
		FileUtils
		CommonUtilities
		Correspondence
		spdlog::spdlog
)

target_compile_features(
		obj_to_graph
		PUBLIC
		cxx_std_11
)