//
// Created by Dave Durbin (Old) on 21/6/21.
//

#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>
#include <Surfel/SurfelBuilder.h>
#include <GeomFileUtils/ObjFileParser.h>

/**
 * Generate a Surfel Graph from an OBJ file
 * Each vertex becomes a Surfel
 * Edges become edges in the graph
 * Normals are either read from file if present or computed from face data
 * if not.
 * Usage:-
 *      obj_to_graph <obj_file>
 */
int main(int argc, char *argv[]) {

  using namespace std;
  using namespace Eigen;

  string infile_name;
  string outfile_name;
  float scale_factor = 1.0f;

  switch (argc) {
    case 3:infile_name = argv[1];
      outfile_name = argv[2];
      break;

    case 5:
      if ((strcmp("-s", argv[1]) == 0) && (argc == 5)) {
        scale_factor = std::stof(argv[2]);
        infile_name = argv[3];
        outfile_name = argv[4];
      } else {
        cout << "Usage : \n\tobj_to_graph [-s scale] <obj_file> <graph_file>" << endl;
        return -1;
      }
      break;

    default:cout << "Usage : \n\tobj_to_graph [-s scale] <obj_file> <graph_file>" << endl;
      return -1;
  }

  cout << "Parsing " << infile_name << endl;
  auto results = animesh::ObjFileParser::parse_file(infile_name, true);
  auto points_with_normals = results.first;
  auto adjacency = results.second;

  // Now generate the graph
  cout << "Generating graph (scale: " << scale_factor << ")" << endl;
  SurfelGraph graph;
  vector<SurfelGraphNodePtr> nodes;
  std::default_random_engine rng{123};
  SurfelBuilder sb(rng);
  size_t vertex_index = 0;

  for (const auto &point_with_normal: points_with_normals) {
    auto surfel = sb.with_id("v_" + to_string(vertex_index))
        ->with_frame(PixelInFrame{1, 1, 0},
                     10,
                     point_with_normal->normal(),
                     point_with_normal->point() * scale_factor)
        ->build();
    auto node = graph.add_node(make_shared<Surfel>(surfel));
    nodes.push_back(node);
    vertex_index++;
    sb.reset();
  }

  set<pair<size_t, size_t>> added_edges;
  for (const auto &adj: adjacency) {
    // Only add each edge once
    if (added_edges.count({adj.first, adj.second}) == 0 &&
        added_edges.count({adj.second, adj.first}) == 0) {
      graph.add_edge(nodes[adj.first], nodes[adj.second], SurfelGraphEdge{1.0});
      added_edges.emplace(adj.first, adj.second);
    }
  }
  save_surfel_graph_to_file(outfile_name, make_shared<SurfelGraph>(graph), false, true);
}