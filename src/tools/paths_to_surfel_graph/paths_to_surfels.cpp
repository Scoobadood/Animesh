//
// Created by Dave Durbin on 29/11/21.
//

#include "paths_to_surfels.h"
#include <vector>
#include <Eigen/Core>
#include <FileUtils/FileUtils.h>

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace Eigen;

    vector<vector<Vector3f>> positions;
    vector<vector<Vector3f>> normals;
    for (auto frameIdx = 0; frameIdx < 10; ++frameIdx) {
        string file_name = "normals_L00_F0" + to_string(frameIdx) + "0.txt";
        vector<Vector3f> normals_for_frame;
        process_file_by_lines(file_name,[normals_for_frame & ](
        const string &line ) {
            line.
            normals.push_back(normals_for_frame);
        });
        string file_name = "pointcloud_L00_F0" + to_string(frameIdx) + "0.txt";
        vector<Vector3f> positions_for_frame;
        process_file_by_lines(file_name,[positions_for_frame & ](
        const string &line ) {
            positions.push_back(positions_for_frame);
        });
    }

    // Load PIF to Frame data

    // Load path data
    //    For each path
    //    For each frame
    //    Generate a frame data from PIF
    //    Stash PIF into
    //    Add surfer to graph
    //    Add neighbours based on PIF data
}