//
// Created by Dave Durbin (Old) on 28/6/21.
//

#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>
#include <map>
#include <fstream>

typedef enum {
    EDGE_TYPE_RED = 1,
    EDGE_TYPE_BLU
} EdgeType;

int main(int argc, const char *argv[]) {
    using namespace std;

    // Load graph
    auto graph = load_surfel_graph_from_file(argv[1]);

    // For each frame
    for (auto frame_index = 0; frame_index < get_num_frames(graph); ++frame_index) {
        ofstream output_file("output.obj", ios::out);
        int vertex_number = 1;
        // Now do the edges with a distance of 1.
        for (const auto &source_node : graph->nodes()) {
            if (source_node->data()->is_in_frame(frame_index)) {
                for (const auto &neighbour_node : graph->neighbours(source_node)) {
                    const auto &edge = graph->edge(source_node, neighbour_node);
                    const auto k_ij = edge->k_ij(frame_index);
                    const auto k_ji = edge->k_ji(frame_index);
                    Eigen::Vector3f src_vertex, src_tangent, src_orth_tangent, src_normal, src_closest_mesh_vertex;
                    source_node->data()->get_all_data_for_surfel_in_frame(frame_index, src_vertex, src_tangent,
                                                                          src_orth_tangent, src_normal,
                                                                          src_closest_mesh_vertex, k_ij);

                    output_file << "v " << src_closest_mesh_vertex.x() << " " << src_closest_mesh_vertex.y() << " "
                                << src_closest_mesh_vertex.z()
                                << endl;
                    ++vertex_number;
                    Eigen::Vector3f dst_vertex, dst_tangent, dst_orth_tangent, dst_normal, dst_closest_mesh_vertex;
                    neighbour_node->data()->get_all_data_for_surfel_in_frame(frame_index, dst_vertex, dst_tangent,
                                                                             dst_orth_tangent, dst_normal,
                                                                             dst_closest_mesh_vertex, k_ji);
                    output_file << "v " << dst_closest_mesh_vertex.x() << " " << dst_closest_mesh_vertex.y() << " "
                                << dst_closest_mesh_vertex.z()
                                << endl;
                    ++vertex_number;

                    const auto &t_ij = edge->t_ij(frame_index);
                    const auto &t_ji = edge->t_ji(frame_index);
                    const auto &t = t_ij + t_ji;
                    if (t.squaredNorm() == 1) {
                        output_file << "l " << (vertex_number - 2) << " " << (vertex_number - 1) << endl;
                    }
                }
            }
        }
        output_file.close();
    }
    return -1;
}