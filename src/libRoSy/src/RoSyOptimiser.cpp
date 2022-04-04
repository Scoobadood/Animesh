#include "RoSy/RoSyOptimiser.h"

#include <set>
#include <vector>
#include <algorithm>
#include <RoSy/RoSy.h>
#include <Properties/Properties.h>
#include <Surfel/SurfelGraph.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Vote/VoteCounter.h>
#include <spdlog/spdlog.h>
#include <iomanip>

RoSyOptimiser::RoSyOptimiser(const Properties &properties, std::default_random_engine &rng)
    : NodeOptimiser(properties, rng) //
{
  setup_termination_criteria(
      "rosy-termination-criteria",
      "rosy-term-crit-relative-smoothness",
      "rosy-term-crit-absolute-smoothness",
      "rosy-term-crit-max-iterations");

  m_damping_factor = m_properties.getFloatProperty("rosy-damping-factor");
  m_weight_for_error = m_properties.getBooleanProperty("rosy-weight-for-error");
  m_weight_for_error_steps = m_properties.getIntProperty("rosy-weight-for-error-steps");
  m_vote_for_best_k = m_properties.getBooleanProperty("rosy-vote-for-best-k");
  m_fix_bad_edges = m_properties.getBooleanProperty("rosy-fix-bad-edges", false);

  setup_ssa();
}

/**
 * @return The total smoothness for a given node in a frame.
 * Also sets the number of neighbours it's compared to so that the mean can be computed.
 */
float
RoSyOptimiser::compute_smoothness_in_frame(
    const SurfelGraph::Edge &edge,
    unsigned int frame_idx) const {

  using namespace Eigen;

  const auto &this_surfel = edge.from()->data();
  Vector3f vertex, normal, tangent;
  this_surfel->get_vertex_tangent_normal_for_frame(frame_idx, vertex, tangent, normal);

  const auto &nbr_surfel = edge.to()->data();
  Vector3f nbr_vertex, nbr_normal, nbr_tangent;
  nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, nbr_vertex, nbr_tangent, nbr_normal);

  // Compute best Ks
  unsigned short k_ij;
  unsigned short k_ji;
  auto best_pair = best_rosy_vector_pair(tangent, normal, k_ij, nbr_tangent, nbr_normal, k_ji);
  float theta = degrees_angle_between_vectors(best_pair.first, best_pair.second);
  const auto smoothness = (theta * theta);
  return smoothness;
}

bool
RoSyOptimiser::compare_worst_first(const SurfelGraphNodePtr &l, const SurfelGraphNodePtr &r) const {
  // Return true if l has higher smoothness : ie is less smooth
  return l->data()->rosy_smoothness() > r->data()->rosy_smoothness();
}

void
RoSyOptimiser::adjust_weights_based_on_error(const std::shared_ptr<Surfel> &s1,
                                             const std::shared_ptr<Surfel> &s2,
                                             float &w_ij,
                                             float &w_ji) const {
  using namespace std;

  static int iterations = 0;

  float smooth1 = s1->rosy_smoothness();
  float smooth2 = s2->rosy_smoothness();

  const auto total_smoothness = smooth1 + smooth2;
  if (total_smoothness != 0) {
    float delta = (smooth1 - smooth2) / total_smoothness;
    // If delta > 0 we want to weight ij less than ji
    delta *= (iterations / (float) m_weight_for_error_steps);
    delta = fminf(0.9f, fmaxf(-0.9f, delta));
    w_ij -= delta;
    w_ji += delta;
  }
  ++iterations;
}

void
RoSyOptimiser::vote_for_best_ks(
    const std::shared_ptr<Surfel> &this_surfel,
    const std::shared_ptr<Surfel> &that_surfel,
    const Eigen::Vector3f &tangent,
    std::vector<unsigned int> &shared_frames,
    unsigned short &best_k_ij,
    unsigned short &best_k_ji) const {
  using namespace std;
  using namespace Eigen;

  // Initialise vote
  VoteCounter<pair<unsigned short, unsigned short>> vote_counter{
      // In the event of a tie pick one at random.
      [&](const vector<pair<unsigned short, unsigned short>> &possibilities) -> pair<int, int> {
        uniform_int_distribution<size_t> i(0, possibilities.size() - 1);
        return possibilities[i(m_random_engine)];
      }
  };

  // For each frame that they both appear in
  for (auto frame_index: shared_frames) {
    // Compute best k_ij/k_ji and tally vote
    unsigned short k_ij, k_ji;
    Vector3f nbr_tan_in_surfel_space;
    Vector3f nbr_norm_in_surfel_space;
    this_surfel->transform_surfel_via_frame(
        that_surfel,
        frame_index,
        nbr_norm_in_surfel_space,
        nbr_tan_in_surfel_space);
    best_rosy_vector_pair(
        tangent,
        Vector3f::UnitY(),
        k_ij,
        nbr_tan_in_surfel_space,
        nbr_norm_in_surfel_space,
        k_ji);
    vote_counter.cast_vote({k_ij, k_ji});
  }
  auto winner = vote_counter.winner();

  best_k_ij = winner.first;
  best_k_ji = winner.second;
}

void
RoSyOptimiser::get_weights(const std::shared_ptr<Surfel> &surfel_a,
                           const std::shared_ptr<Surfel> &surfel_b,
                           float &weight_a,
                           float &weight_b) const {
  weight_a = 1.0f;
  weight_b = 1.0f;
  if (m_weight_for_error) {
    adjust_weights_based_on_error(surfel_a, surfel_b, weight_a, weight_b);
  }
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
Eigen::Vector3f
RoSyOptimiser::optimise_node_with_all_neighbours(const SurfelGraphNodePtr &this_node, //
                                                 const std::shared_ptr<Surfel> &this_surfel,
                                                 Eigen::Vector3f &new_tangent //
) {
  using namespace std;
  using namespace Eigen;

  float weight_sum = 0.0f;

  // For each neighbour of this Surfel
  auto neighbours = m_surfel_graph->neighbours(this_node);
  for (const auto &nbr: neighbours) {
    const auto &nbr_surfel = nbr->data();

    // Select a random frame containing both surfels
    auto shared_frames = get_common_frames(this_surfel, nbr_surfel);
    for (auto frame_idx: shared_frames) {
      Vector3f v1, t1, n1;
      this_surfel->get_vertex_tangent_normal_for_frame(frame_idx, v1, t1, n1);
      Vector3f v2, t2, n2;
      nbr_surfel->get_vertex_tangent_normal_for_frame(frame_idx, v2, t2, n2);

      float this_weight, nbr_weight;
      get_weights(this_surfel, nbr_surfel, this_weight, nbr_weight);

      // Compute the best RoSy pair
      std::pair<Vector3f, Vector3f> best_pair;
      unsigned short k_ij, k_ji;
      best_pair = best_rosy_vector_pair(
          t1, n1, k_ij,
          t2, n2, k_ji);

      weight_sum += this_weight;
      Vector3f v = (best_pair.first * weight_sum) + (best_pair.second * nbr_weight);
      v =this_surfel->transform_for_frame(frame_idx).inverse() * v;
      new_tangent = project_vector_to_plane(v, Vector3f::UnitY()); // Normalizes
      this_surfel->setTangent(new_tangent);
    } // Next frame
  } // Next neighbour
  return new_tangent;
}

/*
 * Smooth an individual Surfel across temporal and spatial neighbours.
 */
void
RoSyOptimiser::optimise_node(const SurfelGraphNodePtr &this_node) {
  using namespace std;
  using namespace Eigen;

  const auto &this_surfel = this_node->data();
  const auto &starting_tangent = this_surfel->tangent();
  Vector3f new_tangent{starting_tangent};


  // Actual Optimisation Method
  new_tangent = optimise_node_with_all_neighbours(this_node, this_surfel, new_tangent);
  //

  // Handle damping
  if (m_damping_factor > 0) {
    new_tangent = (m_damping_factor * starting_tangent) + ((1.0f - m_damping_factor) * new_tangent);
    new_tangent = project_vector_to_plane(new_tangent, Vector3f::UnitY(), true);
  }
  this_surfel->setTangent(new_tangent);
  this_surfel->set_rosy_correction(degrees_angle_between_vectors(starting_tangent, new_tangent));
}

void RoSyOptimiser::store_mean_smoothness(SurfelGraphNodePtr node, float smoothness) const {
  node->data()->set_rosy_smoothness(smoothness);
}

void
RoSyOptimiser::compute_all_dps(
    const std::shared_ptr<Surfel> &s1,
    const std::shared_ptr<Surfel> &s2,
    unsigned int num_frames,
    std::vector<std::vector<std::vector<float>>> &dot_prod,
    std::vector<FrameStat> &frame_stats
) const {
  using namespace Eigen;

  for (int f = 0; f < num_frames; ++f) {
    // Clear dot_prod
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        dot_prod[f][i][j] = 0;
      }
    }
  }

  const auto common_frames = get_common_frames(s1, s2);
  for (const auto f: common_frames) {
    Vector3f v_i, t_i, n_i, v_j, t_j, n_j;
    s1->get_vertex_tangent_normal_for_frame(f, v_i, t_i, n_i);
    s2->get_vertex_tangent_normal_for_frame(f, v_j, t_j, n_j);

    for (int k_ij = 0; k_ij < 4; ++k_ij) {
      for (int k_ji = 0; k_ji < 4; ++k_ji) {
        auto atan_i = vector_by_rotating_around_n(t_i, n_i, k_ij);
        auto atan_j = vector_by_rotating_around_n(t_j, n_j, k_ji);
        auto dp = atan_i.dot(atan_j);
        dot_prod[f][k_ij][k_ji] = dp;

        if (fabsf(dp) > fabsf(frame_stats[f].best_dp)) {
          frame_stats[f].best_dp = dp;
          frame_stats[f].best_delta = (k_ji + 4 - k_ij) % 4;
          frame_stats[f].best_kij = k_ij;
        }
      }
    }
  }
}

void RoSyOptimiser::ended_optimisation() {
  using namespace Eigen;
  using namespace std;

  if( !m_fix_bad_edges) {
    return;
  }
  vector<FrameStat> frame_stats{m_num_frames};
  vector<vector<vector<float>>> dot_prod;
  dot_prod.resize(m_num_frames);
  for (int i = 0; i < m_num_frames; i++) {
    dot_prod[i].resize(4);
    for (int j = 0; j < 4; j++) {
      dot_prod[i][j].resize(4);
    }
  }

  int good_edges = 0;
  int bad_edges = 0;

  // Label graph edges - only works with meshes right now
  // TODO: Fix this to handle edges that don't occur in every frame
  for (const auto &edge: m_surfel_graph->edges()) {
    const auto &s1 = edge.from()->data();
    const auto &s2 = edge.to()->data();

    compute_all_dps(s1, s2, m_num_frames, dot_prod, frame_stats);

    // How many 'best' deltas exist for this edge across all frames?
    set<unsigned short> deltas;
    for (int f = 0; f < m_num_frames; ++f) {
      deltas.emplace(frame_stats[f].best_delta);
    }

    // If one, great, we use it.
    if (deltas.size() == 1) {
      good_edges++;
      if (s1->id() < s2->id()) {
        edge.data()->set_k_low(frame_stats[0].best_kij);
        edge.data()->set_k_high((int) (frame_stats[0].best_kij + frame_stats[0].best_delta) % 4);
      } else {
        edge.data()->set_k_high(frame_stats[0].best_kij);
        edge.data()->set_k_low((int) (frame_stats[0].best_kij + frame_stats[0].best_delta) % 4);
      }
      continue;
    }

    // There are multiple conflicting options for delta for this edge. Dump some stats
    bad_edges++;

    spdlog::warn("Edge : {} --> {}", s1->id(), s2->id());
    ostringstream m;
    m << "  " << deltas.size() << "  options (";
    for (auto d = deltas.begin(); d != deltas.end(); ++d) {
      if (d != deltas.begin())
        m << ", ";
      m << *d;
    }
    m << ")";
    spdlog::warn(m.str());


    //   f    d=0     dp     e       d=1     dp        d=2     dp
    m.str("");
    m.clear();
    m << "    f";
    for (auto d: deltas) {
      m << "    d=" << d << "     dp     e   ";
    }
    spdlog::warn(m.str());

    // For each plausible delta, work out the best kij and dp per frame
    map<unsigned short, pair<int, float>> scores_per_frame[m_num_frames];
    for (int f = 0; f < m_num_frames; ++f) {
      for (auto d: deltas) {
        int best_kij;
        int best_kji;
        float best_dp = 0.0f;
        for (int kij = 0; kij < 4; ++kij) {
          int kji = (kij + d) % 4;
          if (fabsf(dot_prod[f][kij][kji]) > best_dp) {
            best_kij = kij;
            best_kji = kji;
            best_dp = dot_prod[f][kij][kji];
          }
        }
        scores_per_frame[f].emplace(d, make_pair(best_kij, best_dp));
      }
    }

    // Dump these results
    for (int f = 0; f < m_num_frames; ++f) {
      ostringstream s;
      s << "    " << setw(1) << f;
      for (auto d: deltas) {
        s << "   (" << scores_per_frame[f].at(d).first
          << ", "
          << ((scores_per_frame[f].at(d).first + d) % 4) << ")"
          << "  "
          << ((scores_per_frame[f].at(d).second >= 0) ? " " : "")
          << fixed << setw(5) << setprecision(3) << scores_per_frame[f].at(d).second << " [";
        if (d == frame_stats[f].best_delta) {
          s << "****";
        } else {
          float diff = fabsf(fabsf(scores_per_frame[f].at(d).second) - fabsf(frame_stats[f].best_dp));
          s << fixed << setw(4) << setprecision(1) << diff;
        }
        s << "]";
      }
      spdlog::warn("{}", s.str());
    }


    // Vote for the best
    map<unsigned short, float> votes;
    for (auto d: deltas) {
      votes.emplace(d, 0.0f);
    }
    for (int f = 0; f < m_num_frames; ++f) {
      auto sum = 0.0f;
      for (auto d: deltas) {
        sum += fabsf(scores_per_frame[f].at(d).second);
      }
      for (auto d: deltas) {
        auto vote = (sum == 0)
                    ? 0
                    : fabsf(scores_per_frame[f].at(d).second) / sum;
        votes.at(d) = votes.at(d) + vote;
      }
    }
    auto best_d = -1;
    auto best_vote = 0.0f;
    for (const auto &v: votes) {
      if (v.second > best_vote) {
        best_vote = v.second;
        best_d = v.first;
      }
    }
    spdlog::warn("Voted to converge on d={}", best_d);
  }

  spdlog::info("Bad edges {} / {}  {}%",
               bad_edges, (bad_edges + good_edges),
               (100.0 * bad_edges) / (bad_edges + good_edges)
  );
}
