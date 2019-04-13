file(GLOB SOURCES "src/*.cpp")

add_library(Camera STATIC ${SOURCES})

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(Camera 
	PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
	PRIVATE 
		src)

# This makes the project importable from the build directory
export( TARGETS Camera
	FILE CameraLibraryConfig.cmake)


# Every library has unit tests, of course

# GTest requires pthreads
# find_package( Threads REQUIRED )
#find_package( GTest REQUIRED )

# Include specific tests
# include_directories("${GTEST_INCLUDE_DIRS}")

file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable(testCamera
    ${TEST_SOURCES})

target_link_libraries(testCamera 
	Camera 
	gtest 
	gmock)

add_custom_command(
	TARGET testCamera 
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/data ${CMAKE_BINARY_DIR}/camera_test_data
	COMMENT "Copying unit test data."
	DEPENDS testCamera
	)

add_custom_target(copy-runtime-files ALL
    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/runtime-files-dir ${CMAKE_BINARY_DIR}/runtime-files-dir
    )


add_test( NAME FileMissingFocalLengthShouldThrow COMMAND testCamera --gtest_filter=FileMissingFocalLengthShouldThrow)

# Stash it
install(TARGETS testCamera DESTINATION bin)