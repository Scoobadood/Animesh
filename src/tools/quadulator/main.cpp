//
// Created by Dave Durbin (Old) on 28/6/21.
//

#include <Surfel/Surfel_IO.h>
#include <Surfel/SurfelGraph.h>
#include <Quad/Quad.h>
#include <map>
#include <fstream>
#include <Eigen/Geometry>
#include <tclap/CmdLine.h>
#include <tclap/ArgException.h>

struct Args {
    std::string in_file_name;
    std::string save_file_name;
    bool is_ascii;
    float rho;
};

void save_ply_header(const std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>> &graph,
                     const std::string &file_name,
                     bool binary = true,
                     bool little_endian = true) {
    using namespace std;
    using namespace Eigen;

    // Save as PLY
    map<shared_ptr<animesh::Graph<Vector3f, EdgeType>::GraphNode>, int> node_id_to_vertex_index;
    ofstream output_file(file_name, ios::out);
    output_file << "ply" << endl;
    if (binary) {
        output_file << "format binary_";
        if (little_endian) {
            output_file << "little";
        } else {
            output_file << "big";
        }
        output_file << "_endian 1.0" << endl;
    } else {
        output_file << "format ascii 1.0" << endl;
    }
    output_file << "element vertex " << graph->num_nodes() << endl;
    output_file << "property float x" << endl;
    output_file << "property float y" << endl;
    output_file << "property float z" << endl;
    output_file << "element edge " << graph->num_edges() << endl;
    output_file << "property int vertex1" << endl;
    output_file << "property int vertex2" << endl;
    output_file << "property uchar red" << endl;
    output_file << "property uchar green" << endl;
    output_file << "property uchar blue" << endl;
    output_file << "end_header" << endl;
    output_file.close();
}

void save_ply_body_as_bin(const std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>> &graph,
                          const std::string &file_name) {
    using namespace std;
    using namespace Eigen;

    int vertex_index = 0;
    ofstream output_file{file_name, ios_base::app | ios::binary};
    map<const shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>::GraphNode>, int> node_id_to_vertex_index;
    for (const auto &node : graph->nodes()) {
        node_id_to_vertex_index.insert({node, vertex_index});
        float x = node->data().x();
        float y = node->data().y();
        float z = node->data().z();
        output_file.write(reinterpret_cast<char *>(&x), 4);
        output_file.write(reinterpret_cast<char *>(&y), 4);
        output_file.write(reinterpret_cast<char *>(&z), 4);
        vertex_index++;
    }
    for (const auto &edge : graph->edges()) {
        int from = node_id_to_vertex_index.at(edge.from());
        int to = node_id_to_vertex_index.at(edge.to());
        output_file.write(reinterpret_cast<char *>(&from), 4);
        output_file.write(reinterpret_cast<char *>(&to), 4);
        unsigned char blue_red[]{0, 0, 255, 0, 0};
        if (*edge.data() == EDGE_TYPE_RED) {
            output_file.write(reinterpret_cast<char *>(blue_red + 2), 3);
        } else {
            output_file.write(reinterpret_cast<char *>(blue_red), 3);
        }
    }
    output_file.close();
}

void save_ply_body_as_ascii(const std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>> &graph,
                            const std::string &file_name) {
    using namespace std;
    using namespace Eigen;

    ofstream output_file{file_name, ios_base::app};
    map<const shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>::GraphNode>, int> node_id_to_vertex_index;
    int vertex_index = 0;
    for (const auto &node : graph->nodes()) {
        node_id_to_vertex_index.insert({node, vertex_index});
        output_file <<
                    node->data().x() << " " <<
                    node->data().y() << " " <<
                    node->data().z() << endl;
        vertex_index++;
    }
    for (const auto &edge : graph->edges()) {
        output_file << node_id_to_vertex_index.at(edge.from())
                    << " "
                    << node_id_to_vertex_index.at(edge.to())
                    << " ";
        if (*edge.data() == EDGE_TYPE_RED) {
            output_file << "255 0 0" << endl;
        } else {
            output_file << "0 0 255" << endl;
        }
    }
}

void save_as_ply(const std::shared_ptr<animesh::Graph<Eigen::Vector3f, EdgeType>> &graph,
                 const std::string &file_name,
                 bool binary = true,
                 bool little_endian = true) {
    using namespace std;
    using namespace Eigen;

    save_ply_header(graph, file_name, binary, little_endian);

    if (!binary) {
        save_ply_body_as_ascii(graph, file_name);
    } else {
        save_ply_body_as_bin(graph, file_name);
    }
}

Args
parse_args(int argc, char *argv[]) {
    using namespace TCLAP;

    try {
        CmdLine cmd("Command description message", ' ', "0.9");

        SwitchArg ascii_ply("a", "ascii", "Save PLY file as ascii", cmd, false);
        UnlabeledValueArg<std::string> inputFile("in_file.bin",
                                                  "Data will be frad from here",
                                                  true,
                                                  "",
                                                  "infile.graph");
        cmd.add(inputFile);
        UnlabeledValueArg<std::string> outputFile("out_file.ply",
                                                  "Data will be saved here",
                                                  false,
                                                  "object.ply",
                                                  "outfile.ply");

        cmd.add(outputFile);
        ValueArg<float> rho("r", "rho", "Lattice spacing", false, 1.0f, "float");
        cmd.add(rho);

        // Parse the argv array.
        cmd.parse(argc, argv);
        return {inputFile.getValue(), outputFile.getValue(), ascii_ply.getValue(), rho.getValue()};
    } catch (TCLAP::ArgException &e) { // catch any exceptions
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        exit(-1);
    }
}

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace Eigen;

    Args args = parse_args(argc, argv);

    // Load graph
    auto graph = load_surfel_graph_from_file(args.in_file_name);

    // Make an interim graph per frame
    for (auto frame_index = 0; frame_index < get_num_frames(graph); ++frame_index) {
        auto out_graph = build_edge_graph(frame_index, graph, args.rho);

        save_as_ply(out_graph, args.save_file_name, !args.is_ascii);
    }

    return -1;
}