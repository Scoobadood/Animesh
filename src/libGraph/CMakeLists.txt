add_library(
		Graph SHARED
		src/Path.cpp
		include/Graph/Graph.h
		include/Graph/GraphSimplifier.h
		include/Graph/Path.h
		src/Path.cpp)

target_include_directories(
		Graph
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE
		include/Graph)

# Depend on a library that we defined in the top-level file
target_link_libraries(
		Graph
)

add_executable(
		testGraph
		"tests/main.cpp"
		"tests/TestGraph.cpp"
		"tests/TestGraphSimplifier.cpp"
		"tests/TestGraphCycles.cpp"
		${CMAKE_BINARY_DIR}/graph_test_data/cube.obj
		${CMAKE_BINARY_DIR}/graph_test_data/cloth2_1.obj
)

target_link_libraries(
		testGraph
		Graph
		GeomFileUtils
		gtest gmock
		fmt
		${CMAKE_THREAD_LIBS_INIT}
)

add_custom_command(
		OUTPUT
		${CMAKE_BINARY_DIR}/graph_test_data/cube.obj
		${CMAKE_BINARY_DIR}/graph_test_data/cloth2_1.obj
		${CMAKE_BINARY_DIR}/graph_test_data/sphere.obj
		${CMAKE_BINARY_DIR}/graph_test_data/sphere10x10.obj
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data ${CMAKE_BINARY_DIR}/graph_test_data
		COMMENT "Copying graph test data."
		DEPENDS
		tests/test_data/cube.obj
		tests/test_data/cloth2_1.obj
		tests/test_data/sphere.obj
		tests/test_data/sphere10x10.obj
)

add_test(
		NAME TestGraphAddElementShouldMakeSizeLargerByOne
		COMMAND testGraph --gtest_filter=TestGraph.AddElementShouldMakeSizeLargerByOne
)
add_test(
		NAME TestGraphNullEdgeManagerShouldThrow
		COMMAND testGraph --gtest_filter=TestGraph.NullEdgeManagerShouldThrow
)
add_test(
		NAME TestGraphAddElementShouldCallEdgeManager
		COMMAND testGraph --gtest_filter=TestGraph.AddElementShouldCallEdgeManager
)
add_test(
		NAME TestGraphIteratorShouldWork
		COMMAND testGraph --gtest_filter=TestGraph.IteratorShouldWork
)
add_test(
		NAME TestNNEdgeManagerInsertNodeInEmptyListShouldInsertANode
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNodeInEmptyListShouldInsertANode
)
add_test(
		NAME TestNNEdgeManagerInsertNodeInEmptyListShouldInsertTheRightNode
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNodeInEmptyListShouldInsertTheRightNode
)
add_test(
		NAME TestNNEdgeManagerInsertSmallestNodeInListShouldInsertTheNodeAtTheStart
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertSmallestNodeInListShouldInsertTheNodeAtTheStart
)
add_test(
		NAME TestNNEdgeManagerInsertLargestNodeInListShouldInsertTheNodeAtTheEnd
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertLargestNodeInListShouldInsertTheNodeAtTheEnd
)
add_test(
		NAME TestNNEdgeManagerInsertMiddleNodeInListShouldInsertTheNodeInTheMiddle
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertMiddleNodeInListShouldInsertTheNodeInTheMiddle
)
add_test(
		NAME TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthest
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNewNodeToFullNodeRemovesFurthest
)
add_test(
		NAME TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthestAddClosest
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNewNodeToFullNodeRemovesFurthestAddClosest
)
add_test(
		NAME TestNNEdgeManagerInsertNewNodeMakesEachItsNeghbour
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNewNodeMakesEachItsNeghbour
)
add_test(
		NAME TestNNEdgeManagerInsert_113_Updates_111
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_113_Updates_111
)
add_test(
		NAME TestNNEdgeManagerInsert_113_Updates_115
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_113_Updates_115
)
add_test(
		NAME TestNNEdgeManagerInsert_113_Updates_113
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_113_Updates_113
)
add_test(
		NAME TestNNEdgeManagerInsert_112_updates_111
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_111
)
add_test(
		NAME TestNNEdgeManagerInsert_112_updates_113
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_113
)
add_test(
		NAME TestNNEdgeManagerInsert_112_updates_115
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_115
)
add_test(
		NAME TestNNEdgeManagerInsert_112_updates_112
		COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_112
)


# Stash it
install(TARGETS testGraph DESTINATION bin)
