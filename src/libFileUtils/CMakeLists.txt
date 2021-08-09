add_library(FileUtils SHARED
		src/FileUtils.cpp include/FileUtils/FileUtils.h
		)

target_include_directories(
		FileUtils PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE include/FileUtils
)

target_link_libraries(
		FileUtils
		Geom
)

# Tests
add_executable(
		testFileUtils
		tests/main.cpp
		tests/TestFileUtils.h tests/TestFileUtils.cpp
)

target_link_libraries(
		testFileUtils
		FileUtils
		Geom
		Graph
		gtest gmock
		${CMAKE_THREAD_LIBS_INIT}
)

# add_test( NAME TestFileUtil... COMMAND testFileUtil --gtest_filter=TEstFileUtil...TestName  )
add_test(
		NAME FileNameExtensionWithFileAndExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithFileAndExtension
)
add_test(
		NAME FileNameExtensionWithRelPathFileAndExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithRelPathFileAndExtension
)
add_test(
		NAME FileNameExtensionWithAbsPathFileAndExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithAbsPathFileAndExtension
)
add_test(
		NAME FileNameExtensionWithDottedPathFileAndExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithDottedPathFileAndExtension
)
add_test(
		NAME FileNameExtensionWithDottedPathFileNoExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithDottedPathFileNoExtension
)
add_test(
		NAME FileNameExtensionWithDottedPathFileEmptyExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithDottedPathFileEmptyExtension
)
add_test(
		NAME FileNameExtensionWithFileNoExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithFileNoExtension
)
add_test(
		NAME FileNameExtensionWithNoFileExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithNoFileExtension
)
add_test(
		NAME FileNameExtensionWithRelPathFileNoExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithRelPathFileNoExtension
)
add_test(
		NAME FileNameExtensionWithRelPathNoFileExtension
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithRelPathNoFileExtension
)
add_test(
		NAME FileNameExtensionWithNeither
		COMMAND testFileUtils --gtest_filter=FileNameExtensionWithNeither
)

# Stash it
install(
		TARGETS testFileUtils
		DESTINATION bin
)

