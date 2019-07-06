#include <string>
#include <iostream>

//
// Launch with
//    -d <source_files_directory>
//    -o <output_file_name>
//

void
usage(const std::string &prog_name) {
    using namespace std;

    cout << prog_name << " -d <source_files_directory> -o <output_file_name>" << endl;
    exit(-1);
}

void
handle_arg(const char *prog_name, const char *flag, const char *value, std::string &source_directory,
           std::string &output_file_name) {
    using namespace std;

    if (strlen(flag) != 2 || flag[0] != '-') {
        cerr << "Expected flag: " << flag << endl;
        usage(prog_name);
    }
    switch (flag[1]) {
        case 'd':
            source_directory = value;
            break;
        case 'o':
            output_file_name = value;
            break;
        default:
            cerr << "Unknown flag: " << flag[1] << endl;
            usage(prog_name);
    }
}


void
parse_args(int argc, char *argv[], std::string &source_directory, std::string &output_file_name) {
    using namespace std;

    if (argc != 5) {
        cerr << "Missing arguments" << endl;
        usage(argv[0]);
    }
    handle_arg(argv[0], argv[1], argv[2], source_directory, output_file_name);
    handle_arg(argv[0], argv[3], argv[4], source_directory, output_file_name);
}

int
main(int argc, char *argv[]) {
    using namespace std;

    string source_directory;
    string output_file_name;

    parse_args(argc, argv, source_directory, output_file_name);

    cout << "Read data from directory : " << source_directory << endl;
    cout << "Save file as : " << output_file_name << endl;

    return 0;
}
