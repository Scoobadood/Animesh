#include <iostream>
#include <vector>
#include <Properties/Properties.h>

void usage(const std::string &prog_name) {
    std::cout << "usage: " << prog_name << " [source_directory]" << std::endl;
    exit(-1);
}

std::vector<std::vector<float>> load_depth_data(const std::string& source_directory) {
    using namespace std;

    return vector<vector<float>>{};
}

int main(int argc, char *argv[]) {
    using namespace std;


    string property_file_name;
    if (argc == 2) {
        property_file_name = argv[1];
    } else {
        property_file_name = "animesh.properties";
    }

    Properties p{property_file_name};

    string source_directory = p.getProperty("source-directory");
    vector<vector<float>> depth_maps = load_depth_data(source_directory);

    return 0;
}
