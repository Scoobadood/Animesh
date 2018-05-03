file(GLOB SOURCES "src/*.cpp")

add_library(PointCloud STATIC ${SOURCES})


# PCL
find_package(PCL 1.3 REQUIRED)
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

target_include_directories(PointCloud
	PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	${EIGEN3_INCLUDE_DIR}
	${PCL_INCLUDE_DIRS}
	PRIVATE src)


# Depend on a library that we defined in the top-level file
target_link_libraries(PointCloud
	FileUtils
	${PCL_COMMON_LIBRARIES}
	${PCL_FEATURES_LIBRARIES}
	${PCL_IO_LIBRARIES} )


# This makes the project importable from the build directory
export(
	TARGETS PointCloud
	FILE PointCloudLibraryConfig.cmake)


# Every library has unit tests, of course

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})


file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable( testPointCloud
    ${TEST_SOURCES})

target_link_libraries( testPointCloud 
	PointCloud
	FileUtils
	${PCL_COMMON_LIBRARIES}
	${PCL_FEATURES_LIBRARIES}
	${PCL_IO_LIBRARIES} 
	gtest gmock
	${CMAKE_THREAD_LIBS_INIT} )

# Stash it
install(TARGETS testPointCloud DESTINATION bin)