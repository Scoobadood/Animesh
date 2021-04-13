/**
* Generate a set of surfels in the XZ plane so that we can test PosySmoothing.
*/
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>

int main(int argc, const char **argv) {
    SurfelGraphPtr sg = std::make_shared<SurfelGraph>();
    SurfelGraphNodePtr nodes[5][5];

    for (unsigned int x = 0; x < 5; x++) {
        for (unsigned int z = 0; z < 5; z++) {
            // Randomly display the points

            auto sx = (float) x - 0.25f + (float) ((rand() / (RAND_MAX + 1.)) * 0.5);
            auto sy = 0.0f;
            auto sz = (float) z - 0.25f + (float) ((rand() / (RAND_MAX + 1.)) * 0.5);
            auto surfel = std::make_shared<Surfel>(Surfel("s",
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
    for (unsigned int x = 0; x < 5; x++) {
        for (unsigned int z = 0; z < 5; z++) {
            if (x > 0) {
                sg->add_edge(nodes[z][x], nodes[z][x - 1], SurfelGraphEdge{1.0f});
            }
            if (z > 0) {
                sg->add_edge(nodes[z][x], nodes[z - 1][x], SurfelGraphEdge{1.0f});
            }
        }
    }
    save_surfel_graph_to_file("planar_surfels.bin", sg);
}