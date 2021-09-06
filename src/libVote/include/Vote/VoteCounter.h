//
// Created by Dave Durbin (Old) on 6/9/21.
//

#pragma once

#include <map>
#include <vector>

template<typename T>
class VoteCounter {
  std::map<T, int> m_votes;
  std::function<T(const std::vector<T> &)> m_tie_breaker;
public:
  explicit VoteCounter(std::function<T(const std::vector<T> &)> tie_breaker) : m_tie_breaker{tie_breaker} {}
  void cast_vote(T t) {
    if (m_votes.count(t) == 0) {
      m_votes.emplace(t, 1);
    } else {
      m_votes[t] = m_votes[t] + 1;
    }
  }

  T winner() const {
    using namespace std;

    if( m_votes.empty()) {
      throw std::runtime_error("No votes cast.");
    }

    vector<T> winners;
    int max_vote = 0;
    for (const auto &entry: m_votes) {
      if (entry.second > max_vote) {
        winners.clear();
        winners.push_back(entry.first);
        max_vote = entry.second;
      } else if (entry.second == max_vote) {
        winners.push_back(entry.first);
      }
    }
    if (winners.size() == 1) {
      return winners[0];
    }

    return m_tie_breaker(winners);
  }
};