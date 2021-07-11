//
// Created by Dave Durbin (Old) on 21/6/21.
//

#include "obj_to_graph.h"

#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>
#include <Surfel/SurfelBuilder.h>
#include <GeomFileUtils/ObjFileParser.h>

/**
 * Generate a Surfel Graph from an OBJ file
 * Each vertex becomes a Surfel
 * Edges become edges in the graph
 * Normals are either read from file if present or computed from face data
 * Usage:-
 *      obj_to_graph <obj_file>
 */
int main(int argc, char *argv[]) {

    using namespace std;
    using namespace Eigen;

    if (argc < 3) {
        cout << "Usage : \n\tobj_to_graph <obj_file> <graph_file>" << endl;
        return -1;
    }
    string infile_name = argv[1];
    string outfile_name = argv[2];

    cout << "Parsing " << infile_name << endl;
    auto results = animesh::ObjFileParser::parse_file(infile_name, true);
    auto points_with_normals = results.first;
    auto adjacency = results.second;

    // Now generate the graph
    cout << "Generating graph" << endl;
    SurfelGraph graph;
    vector<SurfelGraphNodePtr> nodes;
    std::default_random_engine re(123);
    SurfelBuilder sb(re);
    size_t vertex_index = 0;

    for (const auto &point_with_normal : points_with_normals) {
        auto surfel = sb.with_id("v_" + to_string(vertex_index))
                ->with_frame(PixelInFrame{1, 1, 0},
                             10,
                             point_with_normal->normal(),
                             point_with_normal->point())
                ->build();
        auto node = graph.add_node(make_shared<Surfel>(surfel));
        nodes.push_back(node);
        vertex_index++;
        sb.reset();
    }

    for (const auto &adj : adjacency) {
        graph.add_edge(nodes[adj.first], nodes[adj.second], SurfelGraphEdge{1.0});
    }

    save_surfel_graph_to_file(outfile_name, make_shared<SurfelGraph>(graph));
}