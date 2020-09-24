file(GLOB SOURCES "*.cpp")

# OpenCL
find_package(OpenCL REQUIRED)
IF (APPLE)
	INCLUDE_DIRECTORIES(/System/Library/Frameworks)
	FIND_LIBRARY(OpenCL_LIBRARY OpenCL)
	MARK_AS_ADVANCED(OpenCL_LIBRARY)
ENDIF (APPLE)
add_definitions(-DCL_SILENCE_DEPRECATION)

# Data Generator
add_executable(
		dat_gen
		dat_gen.cpp
		mesh_cl.cpp mesh_cl.hpp
		model_cl.cpp model_cl.hpp
		utility.cpp utility.hpp
		ray_trace.cl)

target_include_directories(
		dat_gen PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(
		dat_gen
		Camera
		FileUtils
		GeomFileUtils
		OpenCL::OpenCL
		${EXTRA_LIBS}
		${ASSIMP_LIBRARY}
)

