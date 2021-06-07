
#include "Surfel_IO.h"
#include "SurfelBuilder.h"
#include "Surfel.h"

#include <GeomFileUtils/io_utils.h>
#include <Graph/Graph.h>
#include <iostream>
#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include "SurfelGraph.h"
/*
	********************************************************************************
	**																			  **
	**					Load and Save    										  **
	**																			  **
	********************************************************************************
*/

/**
 * Save surfel data as binary file to disk
 */
void
save_surfel_graph_to_file(const std::string &file_name,
                          const SurfelGraphPtr &surfel_graph) {
    using namespace std;
    using namespace spdlog;

    info("Saving {:d} surfels from graph to file {:s}", surfel_graph->num_nodes(), file_name);

    ofstream file{file_name, ios::out | ios::binary};
    // Count
    write_unsigned_int(file, surfel_graph->num_nodes());
    for (auto const &surfel : surfel_graph->nodes()) {
        // ID
        write_string(file, surfel->data()->id());
        // FrameData size
        write_unsigned_int(file, surfel->data()->frame_data().size());
        for (auto const &fd : surfel->data()->frame_data()) {
            // PixelInFrame
            write_size_t(file, fd.pixel_in_frame.pixel.x);
            write_size_t(file, fd.pixel_in_frame.pixel.y);
            write_size_t(file, fd.pixel_in_frame.frame);
            write_float(file, fd.depth);

            // Transform
            write_float(file, fd.transform(0, 0));
            write_float(file, fd.transform(0, 1));
            write_float(file, fd.transform(0, 2));
            write_float(file, fd.transform(1, 0));
            write_float(file, fd.transform(1, 1));
            write_float(file, fd.transform(1, 2));
            write_float(file, fd.transform(2, 0));
            write_float(file, fd.transform(2, 1));
            write_float(file, fd.transform(2, 2));

            // Normal
            write_vector_3f(file, fd.normal);

            // Position
            write_vector_3f(file, fd.position);
        }

        const auto neighbours = surfel_graph->neighbours(surfel);
        write_unsigned_int(file, neighbours.size());
        for (const auto &surfel_ptr : neighbours) {
            write_string(file, surfel_ptr->data()->id());
        }
        write_vector_3f(file, surfel->data()->tangent());
        write_vector_2f(file, surfel->data()->reference_lattice_offset());
        write_float(file, surfel->data()->rosy_smoothness());
        write_float(file, surfel->data()->posy_smoothness());
    }
    file.close();
    info(" done.");
}

/**
 * Load surfel data from binary file
 */
SurfelGraphPtr
load_surfel_graph_from_file(const std::string &file_name, bool read_smoothness) {
    using namespace std;
    using namespace spdlog;

    info("Loading surfel graph from file {:s}", file_name);

    SurfelGraphPtr graph = make_shared<SurfelGraph>(false);
    ifstream file{file_name, ios::in | ios::binary};
    if (file.fail()) {
        throw runtime_error("Error reading file " + file_name);
    }

    unsigned int num_surfels = read_unsigned_int(file);
    info("  loading {:d} surfels", num_surfels);
    map<string, vector<string>> neighbours_of_surfel_by_id;
    map<string, SurfelGraphNodePtr> graph_node_by_id;

    std::default_random_engine random_engine{123};
    auto surfel_builder = new SurfelBuilder(random_engine);
    for (unsigned int sIdx = 0; sIdx < num_surfels; ++sIdx) {
        string surfel_id = read_string(file);
        surfel_builder
                ->reset()
                ->with_id(surfel_id);

        unsigned int num_frames = read_unsigned_int(file);
        for (unsigned int fdIdx = 0; fdIdx < num_frames; ++fdIdx) {
            FrameData fd;

            // PixelInFrame
            fd.pixel_in_frame.pixel.x = read_size_t(file);
            fd.pixel_in_frame.pixel.y = read_size_t(file);

            fd.pixel_in_frame.frame = read_size_t(file);
            fd.depth = read_float(file);

            // Transform
            float m[9];
            for (float &mIdx : m) {
                mIdx = read_float(file);
            }
            fd.transform << m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8];

            // Normal
            fd.normal = read_vector_3f(file);

            // Position
            fd.position = read_vector_3f(file);

            surfel_builder->with_frame(fd);
        }

        unsigned int num_neighbours = read_unsigned_int(file);
        vector<string> neighbours;
        neighbours.reserve(num_neighbours);
        for (unsigned int nIdx = 0; nIdx < num_neighbours; ++nIdx) {
            neighbours.push_back(read_string(file));
        }

        const auto tangent = read_vector_3f(file);
        surfel_builder->with_tangent(tangent);

        const auto closest_mesh_vertex_offset = read_vector_2f(file);
        surfel_builder->with_reference_lattice_offset(closest_mesh_vertex_offset);

        auto surfel_ptr = make_shared<Surfel>(surfel_builder->build());

        const auto rosy_smooth = read_smoothness
                                 ? read_float(file)
                                 : 0.0f;
        const auto posy_smooth = read_smoothness
                                 ? read_float(file)
                                 : 0.0f;
        surfel_ptr->set_rosy_smoothness(rosy_smooth);
        surfel_ptr->set_posy_smoothness(posy_smooth);
        auto graph_node = graph->add_node(surfel_ptr);
        neighbours_of_surfel_by_id.emplace(surfel_id, neighbours);
        graph_node_by_id.emplace(surfel_id, graph_node);
    }
    delete surfel_builder;
    file.close();

    // Populate neighbours
    for (auto &graph_node : graph->nodes()) {
        const auto &neighbour_ids = neighbours_of_surfel_by_id.at(graph_node->data()->id());
        for (const auto &neighbour_id : neighbour_ids) {
            const auto neighbour_node = graph_node_by_id.at(neighbour_id);
            graph->add_edge(graph_node, neighbour_node, 1.0);
        }
    }

    info(" done.");
    return graph;
}