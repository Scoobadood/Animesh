add_library(
		GeomFileUtils SHARED
		src/io_utils.cpp include/GeomFileUtils/io_utils.h
		src/ObjFileParser.cpp include/GeomFileUtils/ObjFileParser.h
		src/PgmFileParser.cpp include/GeomFileUtils/PgmFileParser.h
)

target_include_directories(
		GeomFileUtils
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE
		include/GeomFileUtils
)

target_link_libraries(
		GeomFileUtils
		FileUtils
		Geom
)

# Tests
add_executable(
		testGeomFileUtils
		tests/main.cpp
		tests/TestObjFileParser.cpp tests/TestObjFileParser.h
)

target_link_libraries(
		testGeomFileUtils
		FileUtils
		GeomFileUtils
		Geom
		Graph
		gtest gmock
		${CMAKE_THREAD_LIBS_INIT})

# add_test( NAME TestFileUtil... COMMAND testFileUtil --gtest_filter=TEstFileUtil...TestName  )
add_test(
		NAME ParseSphereHasCorrectNodeCount
		COMMAND testGeomFileUtils --gtest_filter=ParseSphereHasCorrectNodeCount
)
add_test(
		NAME ParseSphereTwoNodesHave10Neighbours
		COMMAND testGeomFileUtils --gtest_filter=ParseSphereTwoNodesHave10Neighbours
)
add_test(
		NAME ParseSphereNinetyNodesHave4Neighbours
		COMMAND testGeomFileUtils --gtest_filter=ParseSphereNinetyNodesHave4Neighbours
)

add_test(
		NAME ParseCubeHasCorrectNormals
		COMMAND testGeomFileUtils --gtest_filter=ParseCubeHasCorrectNormals
)

# Stash it
install(
		TARGETS testFileUtils
		DESTINATION bin
)

