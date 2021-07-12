add_library(Quad
		SHARED
		src/Quad.cpp include/Quad/Quad.h
		)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(Quad
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE
		include/Quad)

target_link_libraries(Quad
		Surfel
		)

### Need Eigen3
#find_package(Eigen3 REQUIRED)
#include_directories(${EIGEN3_INCLUDE_DIR})
#
## Stash it
#install(TARGETS testPoSy DESTINATION bin)
#
