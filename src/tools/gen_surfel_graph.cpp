/**
* Generate a set of surfels in the XZ plane so that we can test PosySmoothing.
*/
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <unistd.h>
#include <random>
#include <cstdlib>
#include <Eigen/Geometry>
#include <Surfel/SurfelBuilder.h>

const int DEFAULT_WIDTH = 2;
const int DEFAULT_HEIGHT = 2;


void connectNodes(SurfelGraphPtr &graph, SurfelGraphNodePtr *nodes, int width, int height) {
    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {
            auto nodeIndex = z * width + x;
            if (x > 0) {
                graph->add_edge(nodes[nodeIndex], nodes[nodeIndex - 1], SurfelGraphEdge{1.0f});
            }
            if (z > 0) {
                graph->add_edge(nodes[nodeIndex], nodes[nodeIndex - width], SurfelGraphEdge{1.0f});
            }
            if ((x > 0) && (z > 0)) {
                graph->add_edge(nodes[nodeIndex], nodes[nodeIndex - width - 1], SurfelGraphEdge{1.0f});
            }
            if ((x < width - 1) && (z > 0)) {
                graph->add_edge(nodes[nodeIndex], nodes[nodeIndex - width + 1], SurfelGraphEdge{1.0f});
            }
        }
    }
}

SurfelGraphPtr generate_cylinder(int height, int numSteps, float radius, float percentage) {

    percentage = std::fminf(1.0f, std::fmaxf(0.1f, percentage));
    std::cout << "Generating cylinder " << numSteps << " x " << height << ", radius " << radius << " percent:"
              << percentage << std::endl;

    // Perturbation for vertices.
    static std::default_random_engine e(123);
    auto surfel_builder = new SurfelBuilder(e);
    static std::uniform_real_distribution<> dis(-0.15f, 0.15f);

    auto sg = std::make_shared<SurfelGraph>();

    auto maxTheta = 2 * M_PI * percentage;
    auto thetaStep = maxTheta / numSteps;

    SurfelGraphNodePtr nodes[numSteps * height];
    for (unsigned int step = 0; step < numSteps; ++step) {
        auto theta = step * thetaStep;
        for (unsigned int z = 0; z < height; z++) {
            auto nudgedTheta = theta + dis(e);

            auto sx = radius * cosf(nudgedTheta);
            auto sy = radius * sinf(nudgedTheta);
            auto sz = z + dis(e);

            auto norm = Eigen::Vector3f{sx, sy, 0}.normalized();
            Eigen::Matrix3f tran{};
            tran << 0, 0, 1,
                    norm[0], norm[1], 0,
                    -norm[1], norm[0], 0;
            auto surfel = std::make_shared<Surfel>(
                    surfel_builder
                            ->reset()
                            ->with_id("s_" + std::to_string(step) + "_" + std::to_string(z))
                            ->with_frame({step, z, 0},
                                         10.0f,
                                         norm,
                                         {sx, sy, sz})
                            ->with_tangent(0.0f, 0.0f, 1.0f)
                            ->with_reference_lattice_offset(0, 0)
                            ->build());
            auto node = sg->add_node(surfel);
            nodes[z * numSteps + step] = node;
        }
    }

    delete surfel_builder;

    connectNodes(sg, nodes, numSteps, height);
    return sg;
}

SurfelGraphPtr generate_plane(int width, int height) {
    std::cout << "Generating plane " << width << " x " << height << "." << std::endl;
    SurfelGraphPtr graph = std::make_shared<SurfelGraph>();
    SurfelGraphNodePtr nodes[width * height];

    static std::default_random_engine rng{123};
    auto surfel_builder = new SurfelBuilder(rng);
    static std::uniform_real_distribution<> dis(-0.4f, 0.4f);

    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {

            auto sx = x - (width / 2.0f) + dis(rng);
            auto sy = 0.0f;
            auto sz = z - (height / 2.0f) + dis(rng);
            Eigen::Vector3f norm{0, 1, 0};

            auto surfel = std::make_shared<Surfel>(
                    surfel_builder
                            ->reset()
                            ->with_id("s_" + std::to_string(x) + "_" + std::to_string(z))
                            ->with_frame({x, z, 0},
                                         10.0f,
                                         norm,
                                         {sx, sy, sz})
                            ->with_tangent(1.0f, 0.0f, 0.0f)
                            ->with_reference_lattice_offset(0, 0)
                            ->build());
            auto node = graph->add_node(surfel);
            nodes[z * width + x] = node;
        }
    }
    connectNodes(graph, nodes, width, height);
    return graph;
}

int main(int argc, char *const argv[]) {
    static std::uniform_real_distribution<> dis(-0.4f, 0.4f);
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    float radius = 3.0f;
    int numSteps = 9;
    float percentage = 1.0f;
    int ch;
    bool gen_cylinder = false;
    const char *opts = "w:h:n:r:p:c";
    while ((ch = getopt(argc, argv, opts)) != -1) {
        switch (ch) {
            case 'w':
                width = std::stoi(optarg);
                break;
            case 'h':
                height = std::stoi(optarg);
                break;
            case 'n':
                numSteps = std::stoi(optarg);
                break;
            case 'r':
                radius = std::stof(optarg);
                break;
            case 'p':
                percentage = std::stof(optarg);
                break;
            case 'c':
                gen_cylinder = true;
                break;
        }
    }
    argc -= optind;
    argv += optind;

    SurfelGraphPtr graph;
    if (!gen_cylinder) {
        graph = generate_plane(width, height);
    } else {
        graph = generate_cylinder(height, numSteps, radius, percentage);
    }

    save_surfel_graph_to_file("planar_surfels.bin", graph);
}