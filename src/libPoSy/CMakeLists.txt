add_library(PoSy SHARED
		src/PoSy.cpp include/PoSy/PoSy.h
		src/PoSyOptimiser.cpp include/PoSy/PoSyOptimiser.h
        )


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(PoSy
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE
		include/PoSy)

target_link_libraries(PoSy
		Optimise
		)

add_executable(testPoSy
		tests/main.cpp
		tests/TestPoSy.cpp tests/TestPoSy.h
		tests/TestPoSyOptimiser.cpp tests/TestPoSyOptimiser.h
		tests/TestPoSyQij.cpp tests/TestPoSyQij.h
		tests/TestPoSyLatticeNeighbours.cpp tests/TestPoSyLatticeNeighbours.h
		)

target_link_libraries(testPoSy
		PoSy
		spdlog
		gtest
		gmock)

add_test(NAME MissingPointsShouldThrow COMMAND testPoSy --gtest_filter=MissingPointsShouldThrow)

## Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# Stash it
install(TARGETS testPoSy DESTINATION bin)

