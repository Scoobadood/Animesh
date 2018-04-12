file(GLOB SOURCES "src/*.cpp")

add_library(FileUtil STATIC ${SOURCES})

target_include_directories(
	FileUtil PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE src)

# This makes the project importable from the build directory
export(
	TARGETS FileUtil
	FILE FileUtilLibraryConfig.cmake)

# Tests
file( GLOB TEST_SOURCES "tests/*.cpp" )

add_executable(
	testFileUtil
    ${TEST_SOURCES})

target_link_libraries(
	testFileUtil
	FileUtil
	gtest gmock
	${CMAKE_THREAD_LIBS_INIT} )

# add_test( NAME TestFileUtil... COMMAND testFileUtil --gtest_filter=TEstFileUtil...TestName  )

# Stash it
install(TARGETS testFileUtil DESTINATION bin)

