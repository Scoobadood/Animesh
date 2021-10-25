add_library(MultiResolutionOptimiser STATIC
		src/MultiResolutionOptimiser.cpp include/MultiResolutionOptimiser/MultiResolutionOptimiser.h
		)


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(MultiResolutionOptimiser
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		PRIVATE
		include/MultiResolutionOptimiser
		)

target_link_libraries(MultiResolutionOptimiser
		Optimise
		PoSy
		Properties
		RoSy
		Surfel
		)