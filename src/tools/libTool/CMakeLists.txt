add_library(
		Tool
		SHARED
		src/load_pointcloud.cpp include/Tools/tools.h
)

target_include_directories(
		Tool
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE
		include/Tools
)

target_link_libraries(
		Tool
		CommonUtilities
		FileUtils
)

target_compile_features(
		Tool
		PUBLIC
		cxx_std_11
)