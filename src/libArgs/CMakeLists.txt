file(GLOB SOURCES "src/*.cpp")

add_library(Args STATIC ${SOURCES})

target_include_directories(
	Args PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE src)

# This makes the project importable from the build directory
export(
	TARGETS Args
	FILE ArgsLibraryConfig.cmake)


# Every library has unit tests, of course

file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable(
	testArgs
    ${TEST_SOURCES})

target_link_libraries(
	testArgs 
	Args gtest gmock)


# Stash it
install(TARGETS testArgs 
	DESTINATION bin)

add_test( NAME TestElementZeroLengthVectorsShouldThrow COMMAND testArgs --gtest_filter=TestElement.ZeroLengthVectorsShouldThrow)

