//
// Created by Dave Durbin (Old) on 2/4/21.
//

#include "posy_surfel_graph_geometry_extractor.h"
#include <Geom/Geom.h>
#include <numeric>

#include <Eigen/Geometry>

posy_surfel_graph_geometry_extractor::posy_surfel_graph_geometry_extractor(float rho)
        : m_frame{0}, m_rho{rho} {
}

void
compute_distance_to_neighbours(const SurfelGraphNodePtr &node, const SurfelGraphPtr &graphPtr,
                               unsigned int frame, float &mean_nd, float &min_nd) {
    unsigned int num_neighbours = 0;
    Eigen::Vector3f vertex, normal, tangent, t2, reference_lattice_vertex;
    node->data()->get_vertex_tangent_normal_for_frame(frame, vertex, tangent, normal);

    float min_distance = MAXFLOAT;
    float total_distance = 0.0f;
    for (const auto &n : graphPtr->neighbours(node)) {
        if (!n->data()->is_in_frame(frame)) {
            continue;
        }

        Eigen::Vector3f other_vertex, other_normal, other_tangent;
        n->data()->get_vertex_tangent_normal_for_frame(frame, other_vertex, other_tangent, other_normal);
        const auto dist = (vertex - other_vertex).norm();
        total_distance += dist;
        if (dist < min_distance) {
            min_distance = dist;
        }
        ++num_neighbours;
    }

    mean_nd = num_neighbours == 0
              ? 1.0f
              : total_distance / (float) num_neighbours;
    min_nd = num_neighbours == 0
             ? 1.0f
             : min_distance;
}

std::vector<SurfelGraphNodePtr> get_neighbours_in_frame(
        const SurfelGraphPtr &graphPtr,
        unsigned int frame,
        const SurfelGraphNodePtr &node) {
    using namespace std;

    const auto neighbours = graphPtr->neighbours(node);
    vector<SurfelGraphNodePtr> neighbours_in_frame{};
    for (auto &item : neighbours) {
        if (item->data()->is_in_frame(frame)) {
            neighbours_in_frame.emplace_back(item);
        }
    }
    return neighbours_in_frame;
}


std::vector<size_t>
order_neighbours_around_centroid(
        const Eigen::Vector3f &centroid,
        const Eigen::Vector3f &normal,
        const Eigen::Vector3f &t1,
        const Eigen::Vector3f &t2,
        const std::vector<Eigen::Vector3f> &points
) {
    using namespace std;

    // Make index vector
    vector<size_t> idx(points.size());
    iota(idx.begin(), idx.end(), 0);

    // Generate atan2 for each
    std::vector<double> keys;
    for (const auto &p : points) {
        const auto r = p - centroid;
        const auto t = normal.dot(r.cross(t1));
        const auto u = normal.dot(r.cross(t2));
        keys.emplace_back(std::atan2(u, t));
    }

    stable_sort(begin(idx), end(idx),
                [&keys](const size_t v1_index, const size_t &v2_index) {
                    return keys.at(v1_index) > keys.at(v2_index);
                });
    return idx;
}

/*
 * let c be the center around which the counterclockwise sort is to be performed
 * Sort from here: https://stackoverflow.com/questions/47949485/sorting-a-list-of-3d-points-in-clockwise-order
 */
std::vector<size_t>
order_neighbours(
        const Eigen::Vector3f &normal,
        const Eigen::Vector3f &t1,
        const Eigen::Vector3f &t2,
        const std::vector<Eigen::Vector3f> &points) {
    using namespace std;

    // Compute centroid
    Eigen::Vector3f c = Eigen::Vector3f::Zero();
    for (const auto &p : points) {
        c += p;
    }
    c /= ((float) points.size());
    return order_neighbours_around_centroid(c, normal, t1, t2, points);
}


size_t
find_fan_start(
        const Eigen::Vector3f &vertex,
        const Eigen::Vector3f &normal,
        const std::vector<size_t> &ordered_indices,
        const std::vector<Eigen::Vector3f> &vertices
) {
    using namespace std;
    // Iterate through the triangles until we find one that is invalid
    // or else we get back to the start
    auto tentative_start_index = 0;
    while (tentative_start_index < ordered_indices.size()) {
        auto curr_vert_index = ordered_indices.at(tentative_start_index);
        auto next_vert_index = ordered_indices.at((tentative_start_index + 1) % ordered_indices.size());
        auto curr_vert = vertices.at(curr_vert_index);
        auto next_vert = vertices.at(next_vert_index);
        auto nn = (curr_vert - vertex).cross(next_vert - curr_vert);
        if (nn.dot(normal) >= 0) {
            // Found invalid triangle.
            break;
        }
        tentative_start_index++;
    }
    return (tentative_start_index == ordered_indices.size())
           ? 0
           : (tentative_start_index + 1) % ordered_indices.size();
}


/**
 * RLO is the ref lattice offset. Its coords are in the range [-rho/2 , rho/2)
 * with 0 being a lattice intersection.
 * UV are texture coordinates. The bottom left corner of the texture is the 'centre'
 * of a lattice. We need to convert from RLO to UV such that:
 * -rho/2 --> 1/2
 * -rho/4 --> 3/4
 * -rho/1000 --> 0.999
 * 0 --> 0
 * rho/1000 --> 0.001
 * rho/4 --> 1/4
 * rho/2 --> 1/2
 */
void
rlo_to_uv(const Eigen::Vector2f& rlo, float& u, float& v, float rho) {
    u = ((rlo[0] <= 0.0f ? 0.0f : 1.0f) - rlo[0]) / rho;
    v = ((rlo[1] <= 0.0f ? 0.0f : 1.0f) - rlo[1]) / rho;
}
/**
 * For each vertex in the frame, extract a triangle fan that connects it to its
 * neighbours. Avoid triangles that are malformed.
 *
 * @param graphPtr
 * @param frame
 * @param triangle_fans
 * @param fan_sizes
 */
void extract_triangles_for_frame(const SurfelGraphPtr &graphPtr,
                                 unsigned int frame,
                                 std::vector<float> &triangle_fans,
                                 std::vector<float> &triangle_uvs,
                                 std::vector<unsigned int> &fan_sizes,
                                 float rho) {
    using namespace std;

    for (const auto &node : graphPtr->nodes()) {
        const auto &surfel = node->data();
        if (!surfel->is_in_frame(frame)) {
            continue;
        }
        const auto neighbours_in_frame = get_neighbours_in_frame(graphPtr, frame, node);
        // we only make triangle fans if we have at least one triangle
        if (neighbours_in_frame.size() < 2) {
            continue;
        }

        // Get all metadata for main vertex
        Eigen::Vector3f vertex, normal, tangent, orth_tangent, reference_lattice_vertex;
        surfel->get_all_data_for_surfel_in_frame(frame, vertex, tangent, orth_tangent, normal, reference_lattice_vertex);

        // Extract vertex coordinates for frame
        vector<Eigen::Vector3f> neighbour_coords;
        for (const auto &n : neighbours_in_frame) {
            Eigen::Vector3f nbr_vertex, nbr_normal, nbr_tangent;
            n->data()->get_vertex_tangent_normal_for_frame(frame, nbr_vertex, nbr_tangent, nbr_normal);
            neighbour_coords.emplace_back(nbr_vertex);
        }

        // Sort them CCW around normal, returning indices
        const auto ordered_indices = order_neighbours_around_centroid(
                vertex,
                normal,
                tangent,
                orth_tangent,
                neighbour_coords
        );

        // Find the start of the fan such that all triangles are correct
        auto start_index = find_fan_start(
                vertex,
                normal,
                ordered_indices,
                neighbour_coords
        );

        // Output a triangle_fan
        // Start with vertex (we may remove this later)
        triangle_fans.emplace_back(vertex[0]);
        triangle_fans.emplace_back(vertex[1]);
        triangle_fans.emplace_back(vertex[2]);

        // rlo is [-0.5, 0.5)
        auto rlo = node->data()->reference_lattice_offset();
        // Intersection is at (0,0) in texture coords
        float u,v;
        rlo_to_uv(rlo, u, v, rho);
        triangle_uvs.push_back(u);
        triangle_uvs.push_back(v);

        auto idx_index = start_index;
        unsigned int fan_size = 1;
        for (int i = 0; i < ordered_indices.size(); ++i) {
            auto v1_index = ordered_indices.at(idx_index);
            auto v1 = neighbour_coords.at(v1_index);
            triangle_fans.emplace_back(v1[0]);
            triangle_fans.emplace_back(v1[1]);
            triangle_fans.emplace_back(v1[2]);

            // Compute u and v components of (v1-vertex)
            const auto r = v1 - vertex;
            const auto uv1 = r.dot(tangent);
            const auto uv2 = r.dot(orth_tangent);
            triangle_uvs.push_back(u + uv1);
            triangle_uvs.push_back(v + uv2);

            fan_size++;

            idx_index = (idx_index + 1) % ordered_indices.size();
        }
        fan_sizes.push_back(fan_size);
    }
}

void extract_quads_for_frame(const SurfelGraphPtr &graphPtr,
                             unsigned int frame,
                             std::vector<float> &positions,
                             std::vector<float> &quads,
                             std::vector<float> &normals,
                             std::vector<float> &splat_sizes,
                             std::vector<float> &uvs,
                             float rho,
                             float splat_scale_factor) {

    for (const auto &node : graphPtr->nodes()) {
        const auto &surfel = node->data();
        if (!surfel->is_in_frame(frame)) {
            continue;
        }

        Eigen::Vector3f vertex, normal, tangent, t2, reference_lattice_vertex;
        surfel->get_all_data_for_surfel_in_frame(frame, vertex, tangent, t2, normal, reference_lattice_vertex);

        // Stash the vertex vertex
        positions.push_back(vertex.x());
        positions.push_back(vertex.y());
        positions.push_back(vertex.z());

        // ... and normal ...
        normals.push_back(normal.x());
        normals.push_back(normal.y());
        normals.push_back(normal.z());

        // Scale quad based on proximity to neighbours
        float mean_nd, min_nd;
        compute_distance_to_neighbours(node, graphPtr, frame, mean_nd, min_nd);
        const auto splat_size = min_nd * splat_scale_factor;

        // Size of actual quads, centred on the vertex
        for (auto uv : std::vector<std::tuple<float, float>>{
                {-0.5f * splat_size, -0.5f * splat_size},
                {-0.5f * splat_size, 0.5f * splat_size},
                {0.5f * splat_size,  0.5f * splat_size},
                {0.5f * splat_size,  -0.5f * splat_size},
        }) {
            const auto p = vertex
                           + (std::get<0>(uv) * tangent)
                           + (std::get<1>(uv) * t2);
            quads.push_back(p.x());
            quads.push_back(p.y());
            quads.push_back(p.z());
        }
        splat_sizes.push_back(splat_size);

        // UVs are texture coords
        // ref_lat_offset indicates the centre of the cross.
        // also ref_lat_off is in range [-rho/2 , rho/2)
        // We need to divide by rho and add 0.5 to make them in the range
        // 0->1
        auto texture_offset = surfel->reference_lattice_offset();
        auto norm_texture_offset = texture_offset / rho; // -0.5 to 0.5
        float u, v;
        if (norm_texture_offset[0] < 0.0) {
            u = -norm_texture_offset[0];
        } else {
            u = 1.0f - norm_texture_offset[0];
        }
        if (norm_texture_offset[1] < 0.0) {
            v = -norm_texture_offset[1];
        } else {
            v = 1.0f - norm_texture_offset[1];
        }
        uvs.push_back(u);
        uvs.push_back(v);
    }
}

/**
 * @brief Generate the vertex data needed to render the current frame of this graph.
 * All float vectors are populated in x,y,z coordinate triplets.
 * @param graph The Surfel Graph
 * @param positions a vector into which the Surfel positions will be pushed.
 * @param tangents A Vector of XYZ coordinates for the unit indicative tangent.
 * @param normals A Vector of XYZ coordinates for the unit normal.
 * @param scaleFactor A proposed scaling for the normals and tangents.
 */
void posy_surfel_graph_geometry_extractor::extract_geometry(
        const SurfelGraphPtr &graphPtr,
        std::vector<float> &positions,
        std::vector<float> &quads,
        std::vector<float> &triangle_fans,
        std::vector<float> &triangle_uvs,
        std::vector<unsigned int> &fan_sizes,
        std::vector<float> &normals,
        std::vector<float> &splat_sizes,
        std::vector<float> &uvs
) const {

    positions.clear();
    normals.clear();
    quads.clear();
    triangle_fans.clear();
    triangle_uvs.clear();
    fan_sizes.clear();
    splat_sizes.clear();
    uvs.clear();

    extract_quads_for_frame(graphPtr, m_frame, positions, quads, normals, splat_sizes, uvs, m_rho,
                            m_splat_scale_factor);
    extract_triangles_for_frame(graphPtr, m_frame, triangle_fans, triangle_uvs, fan_sizes, m_rho);
}
