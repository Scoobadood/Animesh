add_library(
		Tool
		SHARED
		src/load_pointcloud.cpp include/Tools/tools.h
		src/FieldOptimiser.cpp include/Tools/FieldOptimiser.h
)

target_include_directories(
		Tool
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/include/Tools
		${EIGEN3_INCLUDE_DIR}
#		INTERFACE
#		include
)

target_include_directories(
		Tool
		PUBLIC
		include
)

target_link_libraries(
		Tool
		CommonUtilities
		FileUtils
		Surfel
		PoSy
		RoSy
)

target_compile_features(
		Tool
		PUBLIC
		cxx_std_11
)