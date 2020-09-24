# Need CPD
find_package(Cpd REQUIRED)

# Need OMP
find_package(OpenMP)
if (NOT TARGET OpenMP::OpenMP_CXX)
	add_library(OpenMP_TARGET INTERFACE)
	add_library(OpenMP::OpenMP_CXX ALIAS OpenMP_TARGET)
	target_compile_options(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS})
	find_package(Threads REQUIRED)
	target_link_libraries(OpenMP_TARGET INTERFACE Threads::Threads)
	target_link_libraries(OpenMP_TARGET INTERFACE ${OpenMP_CXX_FLAGS})
endif ()

# Building Animesh
add_executable(
		animesh
		hierarchical_mesher_cli.cpp
		types.h
)

target_link_libraries(
		animesh
		Camera
		Correspondence
		DepthMap
		FileUtils
		GeomFileUtils
		PoSy
		Properties
		RoSy
		Surfel
		Utilities
		Cpd::Library-C++
)

target_compile_features(animesh
		PUBLIC
		cxx_std_11)

# Building posy_cli
add_executable(
		posy_cli
		posy_cli.cpp
)

target_link_libraries(
		posy_cli
		PoSy
		Properties
		Surfel
		Cpd::Library-C++
)

target_compile_features(
		posy_cli
		PUBLIC
		cxx_std_11
)

add_executable(
		animesh_ui
		AnimeshApplication.h AnimeshApplication.cpp
		types.h
		CrossFieldGLCanvas.h CrossFieldGLCanvas.cpp
		hierarchical_mesher_ui.cpp
)

target_link_libraries(
		animesh_ui
		Camera
		Correspondence
		DepthMap
		FileUtils
		GeomFileUtils
		Properties
		RoSy
		Surfel
		Utilities
		Cpd::Library-C++
		nanogui
		${NANOGUI_EXTRA_LIBS}
)

target_compile_features(
		animesh
		PUBLIC
		cxx_std_11
)

# Unit Tests
# Include specific tests
# include_directories("${GTEST_INCLUDE_DIRS}")

file(GLOB TEST_SOURCES "tests/*.cpp")
add_executable(
		testUtilities
		types.h
		${TEST_SOURCES})

target_link_libraries(testUtilities
		DepthMap
		Properties
		Surfel
		gtest gmock)

#add_test( NAME ZeroLengthVectorsShouldThrow COMMAND testVectorAngles --gtest_filter=ZeroLengthVectorsShouldThrow)

