
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

static const unsigned short FLAGS_MARKER = 0xa9f1; // f1a9 = flag

/**
 * Save surfel data as binary file to disk
 */
void
save_surfel_graph_to_file(const std::string &file_name,
                          const SurfelGraphPtr &surfel_graph,
                          bool save_smoothness,
                          bool save_edges
) {
  using namespace std;
  using namespace spdlog;

  info("Saving {:d} surfels from graph to file {:s} {:s} {:s}",
       surfel_graph->num_nodes(), file_name,
       save_smoothness ? "WITH_SMOOTHNESS" : "",
       save_edges ? "WITH_EDGES" : ""
  );

  ofstream file{file_name, ios::out | ios::binary};

  // Count
  if (save_smoothness || save_edges) {
    write_unsigned_short(file, FLAGS_MARKER);
    unsigned short flags =
        (save_smoothness ? FLAG_SMOOTHNESS : 0)
            | (save_edges ? FLAG_EDGES : 0);
    write_unsigned_short(file, flags);
  }
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
    if (save_smoothness) {
      write_float(file, surfel->data()->rosy_smoothness());
      write_float(file, surfel->data()->posy_smoothness());
    }
  }

  if (save_edges) {
    unsigned int written_edges = 0;
    write_unsigned_int(file, surfel_graph->num_edges());
    for (const auto &edge : surfel_graph->edges()) {
      write_string(file, edge.from()->data()->id());
      write_string(file, edge.to()->data()->id());
      write_float(file, edge.data()->weight());

      // Historical: Used to have multiple k_ij, now only 1
      write_size_t(file, 1);
      write_unsigned_short(file, edge.data()->k_ij());
      write_unsigned_short(file, edge.data()->k_ji());
      const auto tv = edge.data()->t_values();
      write_size_t(file, tv);
      for (auto tvi = 0; tvi < tv; ++tvi) {
        write_vector_2i(file, edge.data()->t_ij(tvi));
        write_vector_2i(file, edge.data()->t_ji(tvi));
      }
      written_edges++;
    }
    spdlog::info("Wrote {} edges", written_edges);
  }
  file.close();
  info(" done.");
}

/**
 * Load surfel data from binary file
 */
SurfelGraphPtr
load_surfel_graph_from_file(const std::string &file_name) {
  unsigned short flags;
  return load_surfel_graph_from_file(file_name, flags);
}

/**
 * Load surfel data from binary file
 */
SurfelGraphPtr
load_surfel_graph_from_file(const std::string &file_name, unsigned short &flags) {
  using namespace std;
  using namespace spdlog;

  info("Loading surfel graph from file {:s}", file_name);
  SurfelGraphPtr graph = make_shared<SurfelGraph>(false);
  ifstream file{file_name, ios::in | ios::binary};
  if (file.fail()) {
    throw runtime_error("Error reading file " + file_name);
  }

  unsigned int num_surfels;
  bool read_edges = false;
  bool read_smoothness = false;
  unsigned short flag_marker = read_unsigned_short(file);
  if (flag_marker == FLAGS_MARKER) {
    flags = read_unsigned_short(file);
    read_edges = ((flags & FLAG_EDGES) == FLAG_EDGES);
    read_smoothness = ((flags & FLAG_SMOOTHNESS) == FLAG_SMOOTHNESS);
  } else {
    flags = 0;
    file.seekg(-sizeof(unsigned short), ios_base::cur);
  }
  num_surfels = read_unsigned_int(file);

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
      for (int i=0; i<9; ++i) {
        m[i] = read_float(file);
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

  if (read_edges) {
    const auto num_edges = read_unsigned_int(file);
    info("  loading {:d} edges", num_edges);

    for (auto edge_index = 0; edge_index < num_edges; ++edge_index) {
      const auto from_node_id = read_string(file);
      auto from_node = graph_node_by_id.at(from_node_id);

      const auto to_node_id = read_string(file);
      auto to_node = graph_node_by_id.at(to_node_id);

      if (graph->has_edge(from_node, to_node)) {
        spdlog::debug("Skipping edge {} from {} to {}, already added",
                      edge_index,
                      from_node->data()->id(),
                      to_node->data()->id());
        continue;
      }

      const auto weight = read_float(file);
      SurfelGraphEdge edge{weight};

      const auto kv = read_size_t(file);
      for (auto kvi = 0; kvi < kv; ++kvi) {
        // sic. We've moved from multiple k_ij to a single value. This will initialise with
        // one value (the last one) while consuming all the data in legacy files.
        // Could have more elegantly done this with seek but this works.
        edge.set_k_ij(read_unsigned_short(file));
        edge.set_k_ji(read_unsigned_short(file));
      }
      const auto tv = read_size_t(file);
      for (auto tvi = 0; tvi < tv; ++tvi) {
        edge.set_t_ij(tvi, read_int(file), read_int(file));
        edge.set_t_ji(tvi, read_int(file), read_int(file));
      }
      graph->add_edge(from_node, to_node, edge);
      spdlog::debug("Added edge {} from {} to {}", edge_index, from_node->data()->id(), to_node->data()->id());
    }
  } else {
    info("  generating edges");

    // Generate edges
    for (auto &graph_node : graph->nodes()) {
      const auto &neighbour_ids = neighbours_of_surfel_by_id.at(graph_node->data()->id());
      for (const auto &neighbour_id : neighbour_ids) {
        const auto neighbour_node = graph_node_by_id.at(neighbour_id);
        if (graph->has_edge(graph_node, neighbour_node)) {
          spdlog::debug("Not generating edge from {} to {}, already added",
                        graph_node->data()->id(),
                        neighbour_node->data()->id());
          continue;
        }
        // DEBUG
        debug("    Generating edge from node ids {:s} to {:s}, node pointers {}, {}, shared pointers {} {}",
              graph_node->data()->id(),
              neighbour_node->data()->id(),
              fmt::ptr(graph_node.get()),
              fmt::ptr(neighbour_node.get()),
              fmt::ptr(graph_node),
              fmt::ptr(neighbour_node)
        );
        graph->add_edge(graph_node, neighbour_node, SurfelGraphEdge{1.0});
      }
    }
  }
  file.close();

  info(" done.");
  return graph;
}
