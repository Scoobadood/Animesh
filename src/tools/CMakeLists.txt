cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

# OBJ File Converter
add_executable(obj_conv
   	obj_converter.cpp)

target_link_libraries(obj_conv
	FileUtils)

target_compile_features(obj_conv
	PUBLIC
		cxx_std_11)

# Perf Tool
add_executable(perf_tool
   	perfTool.cpp)

target_link_libraries(perf_tool
  Args
	Field
	FileUtils
	Graph
	RoSy)

target_compile_features(perf_tool
	PUBLIC
		cxx_std_11)
