# Build a surfel graph from an Obj File.
add_executable(
		obj_to_graph
		obj_to_graph.cpp
)

target_link_libraries(
		obj_to_graph
		Surfel
		FileUtils
		GeomFileUtils
		spdlog::spdlog
)

target_compile_features(
		obj_to_graph
		PUBLIC
		cxx_std_11
)