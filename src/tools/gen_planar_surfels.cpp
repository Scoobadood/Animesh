/**
* Generate a set of surfels in the XZ plane so that we can test PosySmoothing.
*/
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <unistd.h>
#include <random>
#include <cstdlib>
#include <Eigen/Geometry>

const int DEFAULT_WIDTH = 2;
const int DEFAULT_HEIGHT = 2;


int main(int argc, char *const argv[]) {
    static std::default_random_engine e(123);
    static std::uniform_real_distribution<> dis(-0.4f, 0.4f);
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    int ch;
    bool gen_cylinder = false;
    const char *opts = "w:h:c";
    while ((ch = getopt(argc, argv, opts)) != -1) {
        switch (ch) {
            case 'w':
                width = std::stoi(optarg);
                break;
            case 'h':
                height = std::stoi(optarg);
                break;
            case 'c':
                gen_cylinder = true;
                break;
        }
    }
    argc -= optind;
    argv += optind;

    std::cout << "Generating " << width << " x " << height << " surfels." << std::endl;
    SurfelGraphPtr sg = std::make_shared<SurfelGraph>();
    SurfelGraphNodePtr nodes[width][height];
    if (!gen_cylinder) {
        for (unsigned int x = 0; x < width; x++) {
            for (unsigned int z = 0; z < height; z++) {
                auto sx = x - (width / 2.0f);
                auto sy = 0.0f;
                auto sz = z - (height / 2.0f);
                Eigen::Vector3f norm{0, 1, 0};

                auto surfel = std::make_shared<Surfel>(Surfel("s_" + std::to_string(x) + "_" + std::to_string(z),
                                                              {
                                                                      {
                                                                              {x, z, 0},
                                                                              10.0f,
                                                                              Eigen::Matrix3f::Identity(),
                                                                              norm,
                                                                              {sx, sy, sz},
                                                                      }
                                                              },
                                                              {1.0f, 0.0f, 0.0f},
                                                              {0, 0}));
                auto node = sg->add_node(surfel);
                nodes[x][z] = node;
            }
        }
    } else {
        for (unsigned int x = 0; x < width; x++) {
            float theta = (x / (width - 2.0f)) * M_PI;
            for (unsigned int z = 0; z < height; z++) {
                auto sx = 3 * cosf(theta);
                auto sy = 3 * sinf(theta);
                auto sz = z;
                auto norm = Eigen::Vector3f{sx, sy, 0}.normalized();
                Eigen::Matrix3f tran{};
                tran(0,0) = 0;
                tran(0,1) = norm[0];
                tran(0,2) = -norm[1];
                tran(1,0) = 0;
                tran(1,1) = norm[1];
                tran(1,2) = norm[0];
                tran(2,0) = 1;
                tran(2,1) = 0;
                tran(2,2) = 0;
                auto surfel = std::make_shared<Surfel>(
                        Surfel("s_" + std::to_string(x) + "_" + std::to_string(z),
                               {
                                       {
                                               {x, z, 0},
                                               10.0f,
                                               tran,
                                               norm,
                                               {sx, sy, sz},
                                       }
                               },
                               {1.0f, 0.0f, 0.0f},
                               {0, 0}));
                auto node = sg->add_node(surfel);
                nodes[x][z] = node;
            }
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
            if ((x > 0) && (z > 0)) {
                sg->add_edge(nodes[z][x], nodes[z - 1][x - 1], SurfelGraphEdge{1.0f});
            }
            if ((x < width - 1) && (z > 0)) {
                sg->add_edge(nodes[z][x], nodes[z - 1][x + 1], SurfelGraphEdge{1.0f});
            }
        }
    }
    save_surfel_graph_to_file("planar_surfels.bin", sg);
}