//
// Created by Dave Durbin on 2019-07-06.
//

#include <iostream>
#include <fstream>

#include "correspondences_io.h"
#include "io_utils.h"

void
load_correspondences_from_file(const std::string &file_name,
                               std::vector<std::vector<PixelInFrame>> &correspondences) {
    using namespace std;

    cout << "Loading correspondences from " << file_name << "..." << flush;

    ifstream file{file_name, ios::out | ios::binary};
    correspondences.clear();

    // Count
    int num_correspondences = read_unsigned_int( file);
    for( int i=0; i< num_correspondences; ++i ) {
        vector<PixelInFrame> correspondence;

        int num_entries = read_unsigned_int( file);
        correspondence.clear();
        for( int j=0; j<num_entries; ++j) {
            // PixelInFrame
            unsigned int x = read_unsigned_int(file);
            unsigned int y = read_unsigned_int(file);
            unsigned int frame = read_unsigned_int(file);
            correspondence.push_back(PixelInFrame{x, y, frame});
        }
        correspondences.push_back(correspondence);
    }
    file.close();
    cout << " done." << endl;
}

void
save_correspondences_to_file(const std::string &file_name,
                             const std::vector<std::vector<PixelInFrame>> &correspondences) {
    using namespace std;

    cout << "Saving correspondences to " << file_name << "..." << flush;

    ofstream file{file_name, ios::out | ios::binary};
    // Count
    write_unsigned_int( file, correspondences.size());
    for( auto const & correspondence : correspondences) {
        // Number of entries
        write_unsigned_int( file, correspondence.size());
        for( auto const &  pixel_in_frame  : correspondence) {
            // PixelInFrame
            write_unsigned_int( file, pixel_in_frame.x);
            write_unsigned_int( file, pixel_in_frame.y);
            write_unsigned_int( file, pixel_in_frame.frame);
        }
    }
    file.close();
    cout << " done." << endl;
}