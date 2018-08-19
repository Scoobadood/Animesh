cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

# Add the binary tree to the search path for inlcude files
file(GLOB SOURCES "*.cpp")

# Building the executable
add_executable(obj_conv 
   	${SOURCES})

target_link_libraries(obj_conv
	FileUtils)

target_compile_features(obj_conv 
	PUBLIC 
		cxx_std_11)
