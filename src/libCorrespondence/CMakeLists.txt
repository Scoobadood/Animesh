add_library(
		Correspondence SHARED
		src/CorrespondenceIO.cpp include/Correspondence/CorrespondenceIO.h
		include/Correspondence/CorrespondenceCompute.h
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
		Correspondence
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		PRIVATE
		include/Correspondence
)

target_link_libraries(
		Correspondence
		Camera
		DepthMap
		Surfel
)

# Include specific tests
#include_directories("${GTEST_INCLUDE_DIRS}")

add_executable(
		testCorrespondence
		tests/main.cpp
		tests/TestCorrespondence.cpp tests/TestCorrespondence.h
)

target_link_libraries(
		testCorrespondence
		Correspondence
		gtest
		gmock
)

#add_custom_command(
#		TARGET testDepthMap
#		POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data ${CMAKE_BINARY_DIR}/depthmap_test_data
#		COMMENT "Copying unit test data."
#		DEPENDS testDepthMap
#)

#add_test(
#		NAME FileMissingShouldThrow
#		COMMAND testDepthMap --gtest_filter=FileMissingShouldThrow
#)

# Stash it
install(
		TARGETS testCorrespondence
		DESTINATION bin
)
