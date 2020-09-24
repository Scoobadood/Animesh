add_library(
		Utilities SHARED
		src/utilities.cpp include/Utilities/utilities.h
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
		Utilities
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE include/Utilities
)

target_link_libraries(
		Utilities
		Camera
		DepthMap
		Properties
)

## Every library has unit tests, of course
#add_executable(
#		testUtilities
#		tests/main.cpp
#		tests/TestUtilities.h tests/TestUtilities.cpp
#		${CMAKE_BINARY_DIR}/utilities_test_data/*
#)
#
#target_link_libraries(
#		testUtilities
#		Utilities
#		gtest
#		gmock
#)
#
#add_custom_command(
#		OUTPUT ${CMAKE_BINARY_DIR}/utilities_test_data/*
#		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data ${CMAKE_BINARY_DIR}/utilities_test_data
#		COMMENT "Copying utilities test data."
#		DEPENDS tests/test_data/*
#)
#
#add_test(
#		NAME TestSurfelIO.LoadFromTestFile
#		COMMAND testSurfel --gtest_filter=TestSurfelIO.LoadFromTestFile
#		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#)
#
#install(
#		TARGETS testUtilities
#		DESTINATION bin
#)

