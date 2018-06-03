file(GLOB SOURCES "src/*.cpp")

add_library(Graph STATIC ${SOURCES})

target_include_directories(
	Graph PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	${EIGEN3_INCLUDE_DIR}
	PRIVATE src)


# Depend on a library that we defined in the top-level file
target_link_libraries(Graph
    Element)


# This makes the project importable from the build directory
export(
	TARGETS Graph
	FILE GraphLibraryConfig.cmake)


# Every library has unit tests, of course

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})


file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable(
	testGraph
    ${TEST_SOURCES})

target_link_libraries(
	testGraph 
	Graph 
	gtest gmock
	${CMAKE_THREAD_LIBS_INIT} )

add_test( NAME TestGraphAddElementShouldMakeSizeLargerByOne COMMAND testGraph --gtest_filter=TestGraph.AddElementShouldMakeSizeLargerByOne)
add_test( NAME TestGraphNullEdgeManagerShouldThrow COMMAND testGraph --gtest_filter=TestGraph.NullEdgeManagerShouldThrow)
add_test( NAME TestGraphAddElementShouldCallEdgeManager COMMAND testGraph --gtest_filter=TestGraph.AddElementShouldCallEdgeManager)
add_test( NAME TestGraphIteratorShouldWork COMMAND testGraph --gtest_filter=TestGraph.IteratorShouldWork)


add_test( NAME TestNNEdgeManagerInsertNodeInEmptyListShouldInsertANode COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNodeInEmptyListShouldInsertANode)
add_test( NAME TestNNEdgeManagerInsertNodeInEmptyListShouldInsertTheRightNode COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNodeInEmptyListShouldInsertTheRightNode)
add_test( NAME TestNNEdgeManagerInsertSmallestNodeInListShouldInsertTheNodeAtTheStart COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertSmallestNodeInListShouldInsertTheNodeAtTheStart )
add_test( NAME TestNNEdgeManagerInsertLargestNodeInListShouldInsertTheNodeAtTheEnd COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertLargestNodeInListShouldInsertTheNodeAtTheEnd )
add_test( NAME TestNNEdgeManagerInsertMiddleNodeInListShouldInsertTheNodeInTheMiddle COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertMiddleNodeInListShouldInsertTheNodeInTheMiddle )
add_test( NAME TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthest COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNewNodeToFullNodeRemovesFurthest )
add_test( NAME TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthestAddClosest COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNewNodeToFullNodeRemovesFurthestAddClosest )
add_test( NAME TestNNEdgeManagerInsertNewNodeMakesEachItsNeghbour COMMAND testGraph --gtest_filter=TestNNEdgeManager.insertNewNodeMakesEachItsNeghbour )
add_test( NAME TestNNEdgeManagerInsert_113_Updates_111 COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_113_Updates_111 )
add_test( NAME TestNNEdgeManagerInsert_113_Updates_115 COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_113_Updates_115 )
add_test( NAME TestNNEdgeManagerInsert_113_Updates_113 COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_113_Updates_113 )
add_test( NAME TestNNEdgeManagerInsert_112_updates_111 COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_111 )
add_test( NAME TestNNEdgeManagerInsert_112_updates_113 COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_113 )
add_test( NAME TestNNEdgeManagerInsert_112_updates_115 COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_115 )
add_test( NAME TestNNEdgeManagerInsert_112_updates_112 COMMAND testGraph --gtest_filter=TestNNEdgeManager.insert_112_updates_112 )





# Stash it
install(TARGETS testGraph DESTINATION bin)