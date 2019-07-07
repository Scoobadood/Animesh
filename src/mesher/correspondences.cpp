#include <iostream>
#include "correspondences_args.h"
#include "correspondences_io.h"
#include "correspondences_compute.h"
#include "types.h"

//
// Launch with
//    -d <source_files_directory>
//    -o <output_file_name>
//

int
main(int argc, char *argv[]) {
    using namespace std;

    string source_directory;
    string output_file_name;

    parse_args(argc, argv, source_directory, output_file_name);

    cout << "Read data from directory : " << source_directory << endl;
    cout << "Save file as : " << output_file_name << endl;

    vector<vector<PixelInFrame>> correspondences;
    compute_correspondences(source_directory, correspondences);

    save_correspondences_to_file(output_file_name, correspondences);

    return 0;
}
