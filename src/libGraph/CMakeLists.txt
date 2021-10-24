add_library(
		Graph SHARED
		src/Path.cpp
		include/Graph/Graph.h
		include/Graph/GraphEdgeSimplifier.h
		include/Graph/GraphNodeSimplifier.h
		include/Graph/Path.h
		include/Graph/CycleExtractor.h
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
		spdlog::spdlog
)

add_executable(
		testGraph
		tests/main.cpp
		tests/TestGraph.cpp
		tests/DGNodeTests.cpp tests/DGNodeTests.h
		tests/DGEdgeTests.cpp tests/DGEdgeTests.h
		tests/UDGNodeTests.cpp tests/UDGNodeTests.h
		tests/UDGEdgeTests.cpp tests/UDGEdgeTests.h
		tests/TestUtilities.cpp tests/TestUtilities.h
		tests/TestGraphSimplifier.cpp
		tests/TestGraphNodeSimplifier.cpp
		tests/TestGraphCycles.cpp
		tests/TestCycleExtractor.cpp tests/TestCycleExtractor.h
		${CMAKE_BINARY_DIR}/graph_test_data/cube.obj
		${CMAKE_BINARY_DIR}/graph_test_data/cloth2_1.obj
)

target_link_libraries(
		testGraph
		Graph
		GeomFileUtils
		gtest gmock
		${CMAKE_THREAD_LIBS_INIT}
		spdlog::spdlog
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

# TESTS

add_test(
		NAME DirectedGraphNodeTests_add_node_by_data_to_empty_graph_should_increase_node_count
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.add_node_by_data_to_empty_graph_should_increase_node_count
)
add_test(
		NAME DirectedGraphNodeTests_add_duplicate_node_by_data_should_increase_node_count_twice
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.add_duplicate_node_by_data_should_increase_node_count_twice
)
add_test(
		NAME DirectedGraphNodeTests_add_different_nodes_by_data_should_increase_node_count_twice
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.add_different_nodes_by_data_should_increase_node_count_twice
)
add_test(
		NAME DirectedGraphNodeTests_add_node_by_value_to_empty_graph_should_increase_node_count
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.add_node_by_value_to_empty_graph_should_increase_node_count
)
add_test(
		NAME DirectedGraphNodeTests_add_duplicate_node_by_value_should_throw
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.add_duplicate_node_by_value_should_throw
)
add_test(
		NAME DirectedGraphNodeTests_add_node_by_value_to_existing_graph_should_increase_node_count
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.add_node_by_value_to_existing_graph_should_increase_node_count
)
add_test(
		NAME DirectedGraphNodeTests_remove_node_also_removes_edges_from_node
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.remove_node_also_removes_edges_from_node
)
add_test(
		NAME DirectedGraphNodeTests_remove_node_also_removes_edges_to_node
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.remove_node_also_removes_edges_to_node
)
add_test(
		NAME DirectedGraphNodeTests_remove_node_also_removes_all_edges_incident_at_node
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.remove_node_also_removes_all_edges_incident_at_node
)
add_test(
		NAME DirectedGraphNodeTests_remove_missing_node_should_throw
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.remove_missing_node_should_throw
)
add_test(
		NAME DirectedGraphNodeTests_node_with_no_edges_has_no_neighbours
		COMMAND testGraph --gtest_filter=DirectedGraphNodeTests.node_with_no_edges_has_no_neighbours
)


add_test(
		NAME TestGraph_GraphAssignmentWorks
		COMMAND testGraph --gtest_filter=TestGraph.GraphAssignmentWorks
)

add_test(
		NAME TestGraphCycles_IdenticalPathsAreEqual
		COMMAND testGraph --gtest_filter=TestGraphCycles.IdenticalPathsAreEqual
)
add_test(
		NAME TestGraphCycles_IdenticalPathsAreEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.IdenticalPathsAreEquivalent
)
add_test(
		NAME TestGraphCycles_DifferentLengthPathsAreNotEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.DifferentLengthPathsAreNotEquivalent
)
add_test(
		NAME TestGraphCycles_ShiftedPathsAreNotEqual
		COMMAND testGraph --gtest_filter=TestGraphCycles.ShiftedPathsAreNotEqual
)
add_test(
		NAME TestGraphCycles_ShiftedPathsAreNotEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.ShiftedPathsAreNotEquivalent
)
add_test(
		NAME TestGraphCycles_IdenticalCyclesAreEqual
		COMMAND testGraph --gtest_filter=TestGraphCycles.IdenticalCyclesAreEqual
)
add_test(
		NAME TestGraphCycles_IdenticalCyclesAreEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.IdenticalCyclesAreEquivalent
)
add_test(
		NAME TestGraphCycles_ReversedCyclesAreEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.ReversedCyclesAreEquivalent
)
add_test(
		NAME TestGraphCycles_OnceShiftedCyclesAreEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.OnceShiftedCyclesAreEquivalent
)
add_test(
		NAME TestGraphCycles_TwiceShiftedCyclesAreEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.TwiceShiftedCyclesAreEquivalent
)
add_test(
		NAME TestGraphCycles_ReversedOnceShiftedCyclesAreEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.ReversedOnceShiftedCyclesAreEquivalent
)
add_test(
		NAME TestGraphCycles_ReversedTwiceShiftedCyclesAreEquivalent
		COMMAND testGraph --gtest_filter=TestGraphCycles.ReversedTwiceShiftedCyclesAreEquivalent
)
add_test(
		NAME TestGraphCycles_SphereCycleCountIs100
		COMMAND testGraph --gtest_filter=TestGraphCycles.SphereCycleCountIs100
)
add_test(
		NAME TestGraphCycles_CubeCycleCountIs6
		COMMAND testGraph --gtest_filter=TestGraphCycles.CubeCycleCountIs6
)
add_test(
		NAME TestGraphCycles_CubeFaceCycleCountIs8
		COMMAND testGraph --gtest_filter=TestGraphCycles.CubeFaceCycleCountIs8
)
add_test(
		NAME TestGraphCycles_ClothPlaneCycleCountIs256
		COMMAND testGraph --gtest_filter=TestGraphCycles.ClothPlaneCycleCountIs256
)
add_test(
		NAME TestGraphCycles_ClothPlaneFaceCycleCountIs225
		COMMAND testGraph --gtest_filter=TestGraphCycles.ClothPlaneFaceCycleCountIs225
)

add_test(
		NAME TestGraphCycles_CyclesAreCorrect
		COMMAND testGraph --gtest_filter=TestGraphCycles.CyclesAreCorrect
)


add_test(
		NAME TestGraphSimplifier_SimplifyTwoNodeGraphIsNotNull
		COMMAND testGraph --gtest_filter=TestGraphSimplifier.SimplifyTwoNodeGraphIsNotNull
)
add_test(
		NAME TestGraphSimplifier_SimplifyTwoNodeGraphNodeHasOneNode
		COMMAND testGraph --gtest_filter=TestGraphSimplifier.SimplifyTwoNodeGraphNodeHasOneNode
)
add_test(
		NAME TestGraphSimplifier_SimplifyTwoNodeGraphNodeIsMergedCorrectly
		COMMAND testGraph --gtest_filter=TestGraphSimplifier.SimplifyTwoNodeGraphNodeIsMergedCorrectly
)
add_test(
		NAME TestGraphSimplifier_SimplifyTwoNodeGraphMappingPropagatesCorrectly
		COMMAND testGraph --gtest_filter=TestGraphSimplifier.SimplifyTwoNodeGraphMappingPropagatesCorrectly
)
add_test(
		NAME TestGraphSimplifier_SimplifyGraphWithNoEdgesShouldThrow
		COMMAND testGraph --gtest_filter=TestGraphSimplifier.SimplifyGraphWithNoEdgesShouldThrow
)


add_test(
		NAME TestGraphNodeSimplifier_NeighboursOfAMiddleNodeIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.NeighboursOfAMiddleNodeIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_NeighboursOfEmptyListIsEmpty
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.NeighboursOfEmptyListIsEmpty
)
add_test(
		NAME TestGraphNodeSimplifier_NeighboursOfMultipleNodesIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.NeighboursOfMultipleNodesIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_NeighboursOfSpacedNodesIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.NeighboursOfSpacedNodesIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_NeighboursOfALeafNodeIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.NeighboursOfALeafNodeIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_NeighboursOfAnOrphanIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.NeighboursOfAnOrphanIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_SecondNeighboursOfAnOrphanIsEmpty
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.SecondNeighboursOfAnOrphanIsEmpty
)
add_test(
		NAME TestGraphNodeSimplifier_SecondNeighboursOfALeafIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.SecondNeighboursOfALeafIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_SecondNeighboursOfANearLeafIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.SecondNeighboursOfANearLeafIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_SecondNeighboursOfAMiddleNodeIsCorrect
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.SecondNeighboursOfAMiddleNodeIsCorrect
)
add_test(
		NAME TestGraphNodeSimplifier_CollapseMiddleNode
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.CollapseMiddleNode
)
add_test(
		NAME TestGraphNodeSimplifier_CollapseEndNode
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.CollapseEndNode
)
add_test(
		NAME TestGraphNodeSimplifier_CollapseOrphanNode
		COMMAND testGraph --gtest_filter=TestGraphNodeSimplifier.CollapseOrphanNode
)

# Undirected Graph Edge Tests
add_test(
		NAME UndirectedGraphEdgeTests_add_edge_to_empty_graph_should_increase_edge_count
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.add_edge_to_empty_graph_should_increase_edge_count
)
add_test(
		NAME UndirectedGraphEdgeTests_add_edge_to_non_empty_graph_should_increase_edge_count
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.add_edge_to_non_empty_graph_should_increase_edge_count
)
add_test(
		NAME UndirectedGraphEdgeTests_add_edge_creates_forward_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.add_edge_creates_forward_edge
)
add_test(
		NAME UndirectedGraphEdgeTests_add_edge_creates_reverse_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.add_edge_creates_reverse_edge
)
add_test(
		NAME UndirectedGraphEdgeTests_add_duplicate_edge_should_throw
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.add_duplicate_edge_should_throw
)
add_test(
		NAME UndirectedGraphEdgeTests_add_reverse_edge_should_throw
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.add_reverse_edge_should_throw
)
add_test(
		NAME UndirectedGraphEdgeTests_remove_edge_reduces_edge_count
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.remove_edge_reduces_edge_count
)
add_test(
		NAME UndirectedGraphEdgeTests_remove_reverse_edge_reduces_edge_count
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.remove_reverse_edge_reduces_edge_count
)
add_test(
		NAME UndirectedGraphEdgeTests_remove_edge_removes_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.remove_edge_removes_edge
)
add_test(
		NAME UndirectedGraphEdgeTests_remove_edge_removes_reverse_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.remove_edge_removes_reverse_edge
)
add_test(
		NAME UndirectedGraphEdgeTests_remove_reverse_edge_removes_reverse_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.remove_reverse_edge_removes_reverse_edge
)
add_test(
		NAME UndirectedGraphEdgeTests_remove_reverse_edge_removes_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.remove_reverse_edge_removes_edge
)
add_test(
		NAME UndirectedGraphEdgeTests_to_node_of_edge_is_neighbour_of_from_node
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.to_node_of_edge_is_neighbour_of_from_node
)
add_test(
		NAME UndirectedGraphEdgeTests_from_node_is_neighbour_of_to_node
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.from_node_is_neighbour_of_to_node
)
add_test(
		NAME UndirectedGraphEdgeTests_forward_edge_is_retrievable_in_forward_order
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.forward_edge_is_retrievable_in_forward_order
)
add_test(
		NAME UndirectedGraphEdgeTests_forward_edge_is_retrievable_in_backward_order
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.forward_edge_is_retrievable_in_backward_order
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_edge_throws_when_edge_is_missing
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_edge_throws_when_edge_is_missing
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_edge_invokes_node_merge_function
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_edge_invokes_node_merge_function
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_isolated_edge_merges_nodes
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_isolated_edge_merges_nodes
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_any_edge_merges_nodes
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_any_edge_merges_nodes
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_edge_removes_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_edge_removes_edge
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_edge_creates_edges_from_neighbouring_nodes
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_edge_creates_edges_from_neighbouring_nodes
)
add_test(
		NAME UndirectedGraphEdgeTests_node_with_edges_to_node_returns_inbound_edges
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.node_with_edges_to_node_returns_inbound_edges
)
add_test(
		NAME UndirectedGraphEdgeTests_node_with_edges_from_node_returns_outbound_edges
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.node_with_edges_from_node_returns_outbound_edges
)
add_test(
		NAME UndirectedGraphEdgeTests_node_with_edges_from_node_returns_inbound_edges
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.node_with_edges_from_node_returns_inbound_edges
)
add_test(
		NAME UndirectedGraphEdgeTests_node_with_edges_to_node_returns_outbound_edges
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.node_with_edges_to_node_returns_outbound_edges
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_singular_edge_does_not_call_edge_merge_function
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_singular_edge_does_not_call_edge_merge_function
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_edge_invokes_edge_merge_function_on_incident_edges
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_edge_invokes_edge_merge_function_on_incident_edges
)
add_test(
		NAME UndirectedGraphEdgeTests_collapse_edge_adds_new_merged_edge
		COMMAND testGraph --gtest_filter=UndirectedGraphEdgeTests.collapse_edge_adds_new_merged_edge
)

# Directed Graph Edge Tests


add_test(
		NAME DirectedGraphEdgeTests_add_edge_to_empty_graph_should_increase_edge_count
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.add_edge_to_empty_graph_should_increase_edge_count
)
add_test(
		NAME DirectedGraphEdgeTests_add_edge_to_non_empty_graph_should_increase_edge_count
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.add_edge_to_non_empty_graph_should_increase_edge_count
)
add_test(
		NAME DirectedGraphEdgeTests_add_edge_creates_forward_edge
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.add_edge_creates_forward_edge
)
add_test(
		NAME DirectedGraphEdgeTests_add_edge_does_not_create_reverse_edge
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.add_edge_does_not_create_reverse_edge
)
add_test(
		NAME DirectedGraphEdgeTests_add_duplicate_edge_should_throw
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.add_duplicate_edge_should_throw
)
add_test(
		NAME DirectedGraphEdgeTests_add_reverse_edge_should_increase_edge_count
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.add_reverse_edge_should_increase_edge_count
)
add_test(
		NAME DirectedGraphEdgeTests_remove_edge_reduces_edge_count
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.remove_edge_reduces_edge_count
)
add_test(
		NAME DirectedGraphEdgeTests_remove_edge_removes_edge
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.remove_edge_removes_edge
)
add_test(
		NAME DirectedGraphEdgeTests_remove_edge_does_not_remove_reverse_edge
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.remove_edge_does_not_remove_reverse_edge
)
add_test(
		NAME DirectedGraphEdgeTests_remove_reverse_edge_should_throw
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.remove_reverse_edge_should_throw
)
add_test(
		NAME DirectedGraphEdgeTests_to_node_of_edge_is_neighbour_of_from_node
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.to_node_of_edge_is_neighbour_of_from_node
)
add_test(
		NAME DirectedGraphEdgeTests_from_node_is_not_neighbour_of_to_node
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.from_node_is_not_neighbour_of_to_node
)
add_test(
		NAME DirectedGraphEdgeTests_forward_edge_is_retrievable_in_forward_order
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.forward_edge_is_retrievable_in_forward_order
)
add_test(
		NAME DirectedGraphEdgeTests_forward_edge_is_not_retrievable_in_backward_order
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.forward_edge_is_not_retrievable_in_backward_order
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_edge_throws_when_edge_is_missing
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_edge_throws_when_edge_is_missing
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_edge_invokes_node_merge_function
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_edge_invokes_node_merge_function
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_isolated_edge_merges_nodes
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_isolated_edge_merges_nodes
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_any_edge_merges_nodes
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_any_edge_merges_nodes
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_edge_removes_edge
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_edge_removes_edge
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_edge_creates_edges_from_neighbouring_nodes
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_edge_creates_edges_from_neighbouring_nodes
)
add_test(
		NAME DirectedGraphEdgeTests_node_with_edges_to_node_returns_inbound_edges
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.node_with_edges_to_node_returns_inbound_edges
)
add_test(
		NAME DirectedGraphEdgeTests_node_with_edges_from_node_returns_outbound_edges
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.node_with_edges_from_node_returns_outbound_edges
)
add_test(
		NAME DirectedGraphEdgeTests_node_with_edges_from_node_does_not_return_inbound_edges
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.node_with_edges_from_node_does_not_return_inbound_edges
)
add_test(
		NAME DirectedGraphEdgeTests_node_with_edges_to_node_does_not_return_outbound_edges
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.node_with_edges_to_node_does_not_return_outbound_edges
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_singular_edge_does_not_call_edge_merge_function
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_singular_edge_does_not_call_edge_merge_function
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_edge_invokes_edge_merge_function_on_incident_edges
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_edge_invokes_edge_merge_function_on_incident_edges
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_edge_invokes_edge_merge_function_on_inbound_edges
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_edge_invokes_edge_merge_function_on_inbound_edges
)
add_test(
		NAME DirectedGraphEdgeTests_collapse_edge_invokes_edge_merge_function_on_outbound_edges
		COMMAND testGraph --gtest_filter=DirectedGraphEdgeTests.collapse_edge_invokes_edge_merge_function_on_outbound_edges
)

# Stash it
install(TARGETS testGraph DESTINATION bin)
