#include "TestVoteCounter.h"
#include <Vote/VoteCounter.h>
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"

#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) \
      EXPECT_THROW(                                        \
        try { stmt; }                                      \
        catch (const etype& ex) {                          \
          std::string s{ex.what()};                        \
          EXPECT_EQ(s, whatstring);                        \
          throw;                                           \
          } , etype)

void TestVoteCounter::SetUp() {
}

void TestVoteCounter::TearDown() {}

TEST_F(TestVoteCounter, should_throw_if_no_votes_cast
) {
  using namespace std;

  VoteCounter<string> v{[](const vector<string> &entries) { return entries[0]; }};
  EXPECT_THROW_WITH_MESSAGE(
      v.winner(),
      runtime_error,
      "No votes cast."
  );
}

TEST_F(TestVoteCounter, should_return_only_entry_if_one_vote_cast
) {
  using namespace std;

  VoteCounter<string> v{[](const vector<string> &entries) { return entries[0]; }};
  v.cast_vote("b");
  EXPECT_EQ(v
                .
                    winner(),
            "b");
}

TEST_F(TestVoteCounter, should_return_max_votes_if_clear_winner
) {
  using namespace std;

  VoteCounter<string> v{[](const vector<string> &entries) { return entries[0]; }};
  v.cast_vote("a");
  v.cast_vote("b");
  v.cast_vote("a");
  EXPECT_EQ(v
                .
                    winner(),
            "a");
}

TEST_F(TestVoteCounter, should_invoke_tie_breaker_if_tied
) {
  using namespace std;
  using namespace testing;

  MockFunction<string(const vector<string> &entries)> mock_tie_breaker;
  VoteCounter<std::string> v{mock_tie_breaker.AsStdFunction()};
  v.cast_vote("a");
  v.cast_vote("b");
  v.cast_vote("a");
  v.cast_vote("b");

// Check that tie breaker is called
  EXPECT_CALL(mock_tie_breaker, Call(_)
  ).Times(1).
      WillOnce(Return("b")
  );

  EXPECT_EQ(v
                .
                    winner(),
            "b");
}
