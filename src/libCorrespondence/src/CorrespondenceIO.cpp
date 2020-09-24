//
// Created by Dave Durbin on 2019-07-06.
//

#include <iostream>
#include <Surfel/PixelInFrame.h>
#include <GeomFileUtils/io_utils.h>
#include <spdlog/spdlog.h>

#include "CorrespondenceIO.h"

void
load_correspondences_from_file(const std::string &file_name,
                               std::vector<std::vector<PixelInFrame>> &correspondences) {
    using namespace std;

    spdlog::info("Loading correspondences from {:s}", file_name);

    ifstream file{file_name, ios::out | ios::binary};
    if (file.fail()) {
        throw runtime_error("Failed to open file " + file_name);
    }

    auto num_correspondences = read_unsigned_int(file);
    correspondences.clear();
    correspondences.reserve(num_correspondences);

    int count = 0;
    for (unsigned int i = 0; i < num_correspondences; ++i) {
        auto num_entries = read_unsigned_int(file);
        vector<PixelInFrame> correspondence;
        correspondence.reserve(num_entries);

        for (unsigned int j = 0; j < num_entries; ++j) {
            // PixelInFrame
            // TODO: Fix correspondence generation and then come back and fix this.
            // Right now correspondences are generated using frames with non-zero rooted count
            auto frame = read_unsigned_int(file);
            auto x = read_unsigned_int(file);
            auto y = read_unsigned_int(file);
            correspondence.emplace_back(x, y, frame);
        }
        correspondences.push_back(correspondence);
    }
    file.close();
}

void
save_correspondences_to_file(const std::string &file_name,
                             const std::vector<std::vector<PixelInFrame>> &correspondences) {
    using namespace std;

    spdlog::info("Saving correspondences to {:s}", file_name);

    ofstream file{file_name, ios::out | ios::binary};
    // Count
    write_unsigned_int(file, correspondences.size());
    for (auto const &correspondence : correspondences) {
        // Number of entries
        write_unsigned_int(file, correspondence.size());
        for (auto const &pixel_in_frame  : correspondence) {
            // PixelInFrame
            write_unsigned_int(file, pixel_in_frame.frame);
            write_unsigned_int(file, pixel_in_frame.pixel.x);
            write_unsigned_int(file, pixel_in_frame.pixel.y);
        }
    }
    file.close();
}