/**
* Generate a set of surfels in the XZ plane so that we can test PosySmoothing.
*/
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <unistd.h>
#include <stdlib.h>

const int DEFAULT_WIDTH = 2;
const int DEFAULT_HEIGHT = 2;

int main(int argc, char * const argv[]) {
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    int ch;
    const char *opts = "w:h:";
    while ((ch = getopt(argc, argv, opts)) != -1) {
        switch (ch) {
            case 'w':
                width = std::stoi(optarg);
                break;
            case 'h':
                height = std::stoi(optarg);
                break;
        }
    }
    argc -= optind;
    argv += optind;


    std:: cout << "Generating " << width << " x " << height << " surfels." << std::endl;
    SurfelGraphPtr sg = std::make_shared<SurfelGraph>();
    SurfelGraphNodePtr nodes[height][width];

    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {
            // Randomly display the points

            auto sx = (float) x - 0.25f + (float) ((rand() / (RAND_MAX + 1.)) * 0.5);
            auto sy = 0.0f;
            auto sz = (float) z - 0.25f + (float) ((rand() / (RAND_MAX + 1.)) * 0.5);
            auto surfel = std::make_shared<Surfel>(Surfel("s_" + std::to_string(x) + "_" + std::to_string(z),
                                                          {
                                                                  {
                                                                          {x, z, 0},
                                                                          10.0f,
                                                                          Eigen::Matrix3f::Identity(),
                                                                          {0.0f, 1.0f, 0.0f},
                                                                          {sx, sy, sz}
                                                                  }
                                                          },
                                                          {1.0f, 0.0f, 0.0f},
                                                          {0.0f, 0.0f}));
            auto node = sg->add_node(surfel);
            nodes[z][x] = node;
        }
    }
    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {
            if (x > 0) {
                sg->add_edge(nodes[z][x], nodes[z][x - 1], SurfelGraphEdge{1.0f});
            }
            if (z > 0) {
                sg->add_edge(nodes[z][x], nodes[z - 1][x], SurfelGraphEdge{1.0f});
            }
            if ((x > 0) && ( z > 0)) {
                sg->add_edge(nodes[z][x], nodes[z - 1][x-1], SurfelGraphEdge{1.0f});
            }
        }
    }
    save_surfel_graph_to_file("planar_surfels.bin", sg);
}