file(GLOB SOURCES "src/*.cpp")

add_library(FileUtils STATIC ${SOURCES})

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

target_include_directories(
	FileUtils PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	${EIGEN3_INCLUDE_DIR}
	PRIVATE src)


target_link_libraries(
	FileUtils Geom)

# This makes the project importable from the build directory
export(
	TARGETS FileUtils
	FILE FileUtilLibraryConfig.cmake)

# Tests
file( GLOB TEST_SOURCES "tests/*.cpp" )

add_executable(
	testFileUtils
    ${TEST_SOURCES})

target_link_libraries(
	testFileUtils
	FileUtils
	Geom
	Graph
	gtest gmock
	${CMAKE_THREAD_LIBS_INIT} )

# add_test( NAME TestFileUtil... COMMAND testFileUtil --gtest_filter=TEstFileUtil...TestName  )

# Stash it
install(TARGETS testFileUtils DESTINATION bin)

