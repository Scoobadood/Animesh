add_library(PoSy STATIC
		src/PoSy.cpp include/PoSy/PoSy.h)


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(PoSy
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
	PRIVATE
		src)

#target_link_libraries( PoSy
#	Geom)

# This makes the project importable from the build directory
export( TARGETS PoSy
	FILE PoSyLibraryConfig.cmake)

file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable(
		testPoSy
		${TEST_SOURCES})

target_link_libraries(testPoSy
		PoSy
		gtest
		gmock)

add_test( NAME MissingPointsShouldThrow COMMAND testPoSy --gtest_filter=MissingPointsShouldThrow)

## Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Stash it
install(TARGETS testPoSy DESTINATION bin)

