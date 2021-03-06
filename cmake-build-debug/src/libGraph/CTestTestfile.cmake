# CMake generated Testfile for 
# Source directory: /Users/dave/Animesh/src/libGraph
# Build directory: /Users/dave/Animesh/cmake-build-debug/src/libGraph
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestGraphAddElementShouldMakeSizeLargerByOne "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestGraph.AddElementShouldMakeSizeLargerByOne")
set_tests_properties(TestGraphAddElementShouldMakeSizeLargerByOne PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;44;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestGraphNullEdgeManagerShouldThrow "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestGraph.NullEdgeManagerShouldThrow")
set_tests_properties(TestGraphNullEdgeManagerShouldThrow PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;45;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestGraphAddElementShouldCallEdgeManager "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestGraph.AddElementShouldCallEdgeManager")
set_tests_properties(TestGraphAddElementShouldCallEdgeManager PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;46;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestGraphIteratorShouldWork "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestGraph.IteratorShouldWork")
set_tests_properties(TestGraphIteratorShouldWork PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;47;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertNodeInEmptyListShouldInsertANode "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertNodeInEmptyListShouldInsertANode")
set_tests_properties(TestNNEdgeManagerInsertNodeInEmptyListShouldInsertANode PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;50;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertNodeInEmptyListShouldInsertTheRightNode "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertNodeInEmptyListShouldInsertTheRightNode")
set_tests_properties(TestNNEdgeManagerInsertNodeInEmptyListShouldInsertTheRightNode PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;51;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertSmallestNodeInListShouldInsertTheNodeAtTheStart "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertSmallestNodeInListShouldInsertTheNodeAtTheStart")
set_tests_properties(TestNNEdgeManagerInsertSmallestNodeInListShouldInsertTheNodeAtTheStart PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;52;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertLargestNodeInListShouldInsertTheNodeAtTheEnd "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertLargestNodeInListShouldInsertTheNodeAtTheEnd")
set_tests_properties(TestNNEdgeManagerInsertLargestNodeInListShouldInsertTheNodeAtTheEnd PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;53;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertMiddleNodeInListShouldInsertTheNodeInTheMiddle "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertMiddleNodeInListShouldInsertTheNodeInTheMiddle")
set_tests_properties(TestNNEdgeManagerInsertMiddleNodeInListShouldInsertTheNodeInTheMiddle PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;54;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthest "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertNewNodeToFullNodeRemovesFurthest")
set_tests_properties(TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthest PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;55;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthestAddClosest "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertNewNodeToFullNodeRemovesFurthestAddClosest")
set_tests_properties(TestNNEdgeManagerInsertNewNodeToFullNodeRemovesFurthestAddClosest PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;56;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsertNewNodeMakesEachItsNeghbour "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insertNewNodeMakesEachItsNeghbour")
set_tests_properties(TestNNEdgeManagerInsertNewNodeMakesEachItsNeghbour PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;57;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsert_113_Updates_111 "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insert_113_Updates_111")
set_tests_properties(TestNNEdgeManagerInsert_113_Updates_111 PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;58;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsert_113_Updates_115 "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insert_113_Updates_115")
set_tests_properties(TestNNEdgeManagerInsert_113_Updates_115 PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;59;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsert_113_Updates_113 "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insert_113_Updates_113")
set_tests_properties(TestNNEdgeManagerInsert_113_Updates_113 PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;60;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsert_112_updates_111 "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insert_112_updates_111")
set_tests_properties(TestNNEdgeManagerInsert_112_updates_111 PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;61;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsert_112_updates_113 "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insert_112_updates_113")
set_tests_properties(TestNNEdgeManagerInsert_112_updates_113 PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;62;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsert_112_updates_115 "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insert_112_updates_115")
set_tests_properties(TestNNEdgeManagerInsert_112_updates_115 PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;63;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
add_test(TestNNEdgeManagerInsert_112_updates_112 "/Users/dave/Animesh/bin/testGraph" "--gtest_filter=TestNNEdgeManager.insert_112_updates_112")
set_tests_properties(TestNNEdgeManagerInsert_112_updates_112 PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libGraph/CMakeLists.txt;64;add_test;/Users/dave/Animesh/src/libGraph/CMakeLists.txt;0;")
