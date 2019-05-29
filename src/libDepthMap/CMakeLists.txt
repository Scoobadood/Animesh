file(GLOB SOURCES "src/*.cpp")

add_library(DepthMap STATIC ${SOURCES})

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(DepthMap 
	PUBLIC 
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
	PRIVATE 
		src)

target_link_libraries( DepthMap 
	FileUtils
)

# This makes the project importable from the build directory
export( TARGETS DepthMap
	FILE DepthMapLibraryConfig.cmake)


# GTest requires pthreads
# find_package( Threads REQUIRED )
# find_package( GTest REQUIRED )

# Include specific tests
# include_directories("${GTEST_INCLUDE_DIRS}")

file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable(testDepthMap
    ${TEST_SOURCES})

target_link_libraries(testDepthMap 
	DepthMap 
	gtest 
	gmock)

add_custom_command(
	TARGET testDepthMap 
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data ${CMAKE_BINARY_DIR}/depthmap_test_data
	COMMENT "Copying unit test data."
	DEPENDS testDepthMap
	)

# add_custom_target(copy-runtime-files ALL
#     COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/runtime-files-dir ${CMAKE_BINARY_DIR}/runtime-files-dir
#     )


add_test( NAME FileMissingShouldThrow COMMAND testDepthMap --gtest_filter=FileMissingShouldThrow)

# Stash it
install(TARGETS testDepthMap DESTINATION bin)