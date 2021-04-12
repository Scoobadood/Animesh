add_library(Optimise STATIC
		src/SurfelSelectionAlgorithm.cpp include/Optimise/SurfelSelectionAlgorithm.h
		)


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(Optimise
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		PRIVATE
		include/Optimise)

#target_link_libraries(Optimise
#)