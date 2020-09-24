add_library(
		Field SHARED
		include/Field/FieldOptimiser.h src/FieldOptimiser.cpp
)

target_include_directories(
		Field PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE incelud/Field
)

target_link_libraries(
		Field
		FileUtils
		RoSy
		Graph
)

# Tests
add_executable(
		testField
		tests/main.cpp
)

target_link_libraries(testField
		Field
		FileUtils
		Graph
		gtest
		gmock
		${CMAKE_THREAD_LIBS_INIT}
		)

# Stash it
install(
		TARGETS testField
		DESTINATION bin
)

