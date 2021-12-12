//
// Created by Dave Durbin on 29/11/21.
//

#include <vector>
#include <Eigen/Core>
#include <FileUtils/FileUtils.h>
#include <CommonUtilities/split.h>
#include <fstream>
#include <Correspondence/CorrespondenceIO.h>
#include <Surfel/SurfelBuilder.h>
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>

std::string file_name_for_frame_and_level(std::string root, unsigned int frame, unsigned int level) {
    assert(frame < 100);
    assert (level < 100);
    std::ostringstream oss;
    oss << root;
    oss << "_L";
    if( level < 10) oss << "0";
    oss << level;
    oss << "_F";
    if (frame < 10) oss << "0";
    oss << frame;
    oss << ".txt";
    return oss.str();
}



const int LEVEL = 3;

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace Eigen;

    vector<vector<Vector3f>> surfel_locations;
    vector<vector<Vector3f>> normals;
    map<PixelInFrame, size_t> pifs;
    const int NUM_FRAMES = 4;
    for (unsigned int frameIdx = 0; frameIdx < NUM_FRAMES; ++frameIdx) {
        string file_name = file_name_for_frame_and_level("normals", frameIdx, LEVEL);
        vector<Vector3f> normals_for_frame;
        process_file_by_lines(file_name, [&](
                const string &line) {
            Vector3f v = string_to_vec3f(line);
            normals_for_frame.push_back(v);
        });
        normals.push_back(normals_for_frame);

        file_name = file_name_for_frame_and_level("pointcloud", frameIdx, LEVEL);
        vector<Vector3f> positions_for_frame;
        process_file_by_lines(file_name, [&](
                const string &line) {
            Vector3f v = string_to_vec3f(line);
            positions_for_frame.push_back(v);
        });
        surfel_locations.push_back(positions_for_frame);

        file_name = file_name_for_frame_and_level("pifs", frameIdx, LEVEL);
        size_t line_no = 0;
        process_file_by_lines(file_name, [&](
                const string &line) {
            auto values = line.substr(1, line.size() - 2);
            auto coords = split(values, ',');
            unsigned int px = stoi(coords[0]);
            unsigned int py = stoi(coords[1]);
            pifs.emplace(PixelInFrame{px, py, frameIdx}, line_no);
            ++line_no;
        });
    }

    // Load path data
    string file_name = "level_0"+to_string(LEVEL)+"_corr.bin";
    vector<vector<PixelInFrame>> correspondences;
    load_correspondences_from_file(file_name, correspondences);

    map<PixelInFrame, SurfelGraphNodePtr> pif_to_surfel;
    default_random_engine re{123};
    auto sb = new SurfelBuilder(re);
    auto *graph = new SurfelGraph();
    //    For each path
    unsigned int surfel_id = 0;
    for (const auto &correspondence: correspondences) {
        // Make a surfel
        sb->reset();
        string surfel_name = "s" + to_string(surfel_id);
        sb->with_id(surfel_name);

        //    For each frame
        for (const auto &pif: correspondence) {
            // Make a Frame Data and add it

            //    Generate a frame data from PIF
            int index = pifs.at(pif);
            sb->with_frame(pif, 0, normals[pif.frame][index], surfel_locations[pif.frame][index]);
        }
        //    Add surfel to graph
        auto node = graph->add_node(make_shared<Surfel>(sb->build()));
        for (const auto &pif: correspondence) {
            pif_to_surfel.emplace(pif, node);
        }

        ++surfel_id;
    }

    //    Add neighbours based on PIF data
    for (const auto &node: graph->nodes()) {
        for (const auto &fd: node->data()->frame_data()) {
            const auto &pif = fd.pixel_in_frame;
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    PixelInFrame check{pif.pixel.x + dx, pif.pixel.y + dy, pif.frame};
                    const auto &n = pif_to_surfel.find(check);
                    if (n != pif_to_surfel.end()) {
                        try {
                            graph->add_edge(node, n->second, SurfelGraphEdge{1});
                        } catch(std::runtime_error const & err ) {
                        }
                    }
                }
            }
        }
    }

    save_surfel_graph_to_file("sfl_0"+to_string(LEVEL)+".bin", static_cast<const SurfelGraphPtr>(graph));
}