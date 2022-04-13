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

target_link_libraries(
		Quad
		Geom
		PoSy
		RoSy
		Surfel
)

