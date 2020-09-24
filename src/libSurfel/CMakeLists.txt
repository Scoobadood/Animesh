add_library(
		Surfel SHARED
		src/Surfel.cpp include/Surfel/Surfel.h
		src/Pixel.cpp include/Surfel/Pixel.h
		src/PixelInFrame.cpp include/Surfel/PixelInFrame.h
		src/FrameData.cpp include/Surfel/FrameData.h
		src/Surfel_Compute.cpp include/Surfel/Surfel_Compute.h
		src/Surfel_IO.cpp include/Surfel/Surfel_IO.h
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
		Surfel
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE include/Surfel
)

target_link_libraries(
		Surfel
		DepthMap
		Geom
		GeomFileUtils
		Graph
		Properties
		fmt
)

# Every library has unit tests, of course
add_executable(
		testSurfel
		tests/main.cpp
		tests/TestSurfel.h tests/TestSurfel.cpp
		${CMAKE_BINARY_DIR}/surfel_test_data/gold_graph.bin
)

target_link_libraries(
		testSurfel
		Surfel
		gtest
		gmock
)

add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/surfel_test_data/gold_graph.bin
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data ${CMAKE_BINARY_DIR}/surfel_test_data
		COMMENT "Copying surfel test data."
		DEPENDS tests/test_data/gold_graph.bin
)

add_test(
		NAME TestSurfelIO.LoadFromTestFile
		COMMAND testSurfel --gtest_filter=TestSurfelIO.LoadFromTestFile
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_test(
		NAME TestSurfelIO.SaveToTestFile
		COMMAND testSurfel --gtest_filter=TestSurfelIO.SaveToTestFile
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_test(
		NAME TestSurfelIO.SaveLoadRoundTrip
		COMMAND testSurfel --gtest_filter=TestSurfelIO.SaveLoadRoundTrip
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)


# Stash it
install(
		TARGETS testSurfel
		DESTINATION bin
)

