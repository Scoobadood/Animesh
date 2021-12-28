# DM to Graph constructs a Surfel Graph from a set of depthmaps
# Given camera intrinsic and extrinsic variables
# Configured by the DM section of the animesh.properties file
add_executable(
		dm_to_graph
		main.cpp
)

target_link_libraries(
		dm_to_graph
		CommonUtilities
		Correspondence
		FileUtils
		Properties
		Surfel
		Tool
)

target_compile_features(
		dm_to_graph
		PUBLIC
		cxx_std_11
)