add_library(
		Vote SHARED
		include/Vote/VoteCounter.h
)

set_target_properties(
		Vote
		PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(
		Vote
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PRIVATE
		include/Vote)

add_executable(
		testVoteCounter
		tests/main.cpp
		tests/TestVoteCounter.h tests/TestVoteCounter.cpp
)

target_link_libraries(
		testVoteCounter
		Vote
		gtest gmock
		${CMAKE_THREAD_LIBS_INIT}
		spdlog::spdlog
)

# TESTS
add_test(
		NAME TestVoteCounter_should_throw_if_no_votes_cast
		COMMAND testVoteCounter --gtest_filter=TestVoteCounter.should_throw_if_no_votes_cast
)

add_test(
		NAME TestVoteCounter_should_return_only_entry_if_one_vote_cast
		COMMAND testVoteCounter --gtest_filter=TestVoteCounter.should_return_only_entry_if_one_vote_cast
)

add_test(
		NAME TestVoteCounter_should_return_max_votes_if_clear_winner
		COMMAND testVoteCounter --gtest_filter=TestVoteCounter.should_return_max_votes_if_clear_winner
)
add_test(
		NAME TestVoteCounter_should_invoke_random_picker_if_tied
		COMMAND testVoteCounter --gtest_filter=TestVoteCounter.should_invoke_random_picker_if_tied
)


# Stash it
install(TARGETS testVoteCounter DESTINATION bin)
