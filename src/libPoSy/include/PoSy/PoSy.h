#include <vector>
#include <Eigen/Core>

/**
 * Given two sets of 3D positions, return the indices and values of the closest two along with the distance between them.
 */
std::tuple<size_t, size_t, Eigen::Vector3f, Eigen::Vector3f, float>
closest_points(const std::vector<Eigen::Vector3f>& points_a, const std::vector<Eigen::Vector3f>& points_b );


/**
 * Given a point in space, a normal, tangent vector and rho (lattice spacing) compute the 8 nearest vertices on the lattice
 */
 std::vector<Eigen::Vector3f> compute_local_lattice_vertices(
         const Eigen::Vector3f& position,
         const Eigen::Vector3f& unit_normal,
         const Eigen::Vector3f& tangent,
         float rho );

 /**
  * Compute the new update for the given node
  */
 Eigen::Vector3f
 average_posy_vectors(const Eigen::Vector3f &p1,
                      const Eigen::Vector3f &o1,
                      const Eigen::Vector3f &n1,
                      const Eigen::Vector3f &p2,
                      const Eigen::Vector3f &o2,
                      const Eigen::Vector3f &n2,
                      float rho,
                      float weight
 );

