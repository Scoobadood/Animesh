# Add the binary tree to the search path for inlcude files
file(GLOB SOURCES "*.cpp")

# PCL
find_package(PCL 1.3 REQUIRED)
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

# Building the executable
add_executable(CrossTemp ${SOURCES})


target_include_directories(CrossTemp
	PUBLIC 
		${PCL_INCLUDE_DIRS}
	PRIVATE
		src)

target_link_libraries(CrossTemp 
	Args
	FileUtils
	Graph
	${PCL_COMMON_LIBRARIES}
	${PCL_FEATURES_LIBRARIES}
	${PCL_IO_LIBRARIES} )

target_compile_features(CrossTemp PUBLIC cxx_std_11)
