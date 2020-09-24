add_library(
		Camera SHARED
		src/Camera.cpp include/Camera/Camera.h
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
		Camera
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE
		include/Camera
)

add_executable(
		testCamera
		tests/main.cpp
		tests/TestCamera.cpp tests/TestCamera.h
		tests/camera_test_data/bad_camera_1.txt
		tests/camera_test_data/one_triangle_mesh.obj
		tests/camera_test_data/tri_camera.txt
		tests/camera_test_data/one_triangle_mesh.obj
)

target_link_libraries(
		testCamera
		Camera
		gtest
		gmock)

add_custom_command(
		TARGET testCamera
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/camera_test_data ${CMAKE_BINARY_DIR}/camera_test_data
		COMMENT "Copying unit test data."
		DEPENDS testCamera
)

#add_custom_target(copy-runtime-files ALL
#    COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/runtime-files-dir ${CMAKE_BINARY_DIR}/runtime-files-dir
#    )

add_test(
		NAME FileMissingFocalLengthShouldThrow
		COMMAND testCamera --gtest_filter=FileMissingFocalLengthShouldThrow
)
add_test(
		NAME BackprojectOriginFrom_0_0_10
		COMMAND testCamera --gtest_filter=BackprojectOriginFrom_0_0_10
)
add_test(
		NAME BackprojectOriginFrom_5_0_0
		COMMAND testCamera --gtest_filter=BackprojectOriginFrom_5_0_0
)
add_test(
		NAME BackprojectOriginFrom_m5_0_0
		COMMAND testCamera --gtest_filter=BackprojectOriginFrom_m5_0_0
)
add_test(
		NAME BackprojectBottomLeftFrom_10_0_0
		COMMAND testCamera --gtest_filter=BackprojectBottomLeftFrom_10_0_0
)
add_test(
		NAME depth_to_pixel_roundtrip
		COMMAND testCamera --gtest_filtedepth_to_pixel_roundtrip
		r=)
add_test(
		NAME depth_to_pixel_roundtrip_at_multiple_resolutions
		COMMAND testCamera --gtest_filter=depth_to_pixel_roundtrip_at_multiple_resolutions
)
add_test(
		NAME multiple_resolutions_yield_same_depth
		COMMAND testCamera --gtest_filter=multiple_resolutions_yield_same_depth
)


# Stash it
install(
		TARGETS testCamera
		DESTINATION bin
)