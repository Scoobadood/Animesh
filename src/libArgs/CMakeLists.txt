add_library(
		Args SHARED
		src/Args.cpp include/Args/Args.h
)

target_include_directories(
		Args PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PRIVATE
		include/Args
		include/tclap
)

add_executable(
		testArgs
		tests/main.cpp
		tests/TestArgs.cpp
)

target_link_libraries(
		testArgs
		Args
		gtest
		gmock
)


# Stash it
install(
		TARGETS testArgs
		DESTINATION bin
)

add_test(
		NAME TestElementZeroLengthVectorsShouldThrow
		COMMAND testArgs --gtest_filter=TestElement.ZeroLengthVectorsShouldThrow
)

