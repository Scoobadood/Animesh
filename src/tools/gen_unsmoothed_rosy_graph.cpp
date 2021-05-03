#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <random>
#include <cstdlib>
#include <Eigen/Geometry>
#include <Geom/Geom.h>
#include <tclap/CmdLine.h>


/*
 * Generate an unsmoothed RoSy graph.
 * Invoke as follows:
 *    --type, -t  planar | sphere | sinusoid | cylinder
 *    --radius, -r (for sphere and cylinder only)
 *    --height, -h Height. (For cylinder and planar only)
 *    --width, -w Width. For planar only.
 *    --perturb_position
 *    --perturb_orientation
 */

const int DEFAULT_WIDTH = 10;
const int DEFAULT_HEIGHT = 10;
struct Args {
    enum type {
        PLANAR,
        SPHERICAL,
    };

    type m_type;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_rings;
    unsigned int m_segments;
    float m_radius;
    bool m_perturb_orientation;
    bool m_perturb_position;
    std::string m_output_file_name;
};


/*
 * Each node in the graph is connected to its 8 surrounding neighbours.
 * We rely on the fact that the graph is not directed so only create edges
 * from (x,y) to (x-1, y-1), (x, y-1), (x+1, y-1) and (x-1,y).
 */
void connectNodes(SurfelGraphPtr &graph,
                  SurfelGraphNodePtr *nodes,
                  unsigned int width,
                  unsigned int height) {
    for (auto x = 0; x < width; x++) {
        for (auto z = 0; z < height; z++) {
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

SurfelGraphPtr generate_plane(const Args &args) {
    const auto width = args.m_width;
    const auto height = args.m_height;
    std::cout << "Generating plane " << width << " x " << height << "." << std::endl;

    SurfelGraphPtr graph = std::make_shared<SurfelGraph>();
    SurfelGraphNodePtr nodes[width * height];

    std::default_random_engine e(123);

    std::uniform_real_distribution<> p_dis(-0.4f, 0.4f);
    std::uniform_real_distribution<> r_dis(-M_PI_4, M_PI_4);

    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {


            Eigen::Vector3f norm{0, 1, 0};

            Eigen::Vector3f pos{
                    (float) (x - (width / 2.0) + (args.m_perturb_position ? p_dis(e) : 0.0)),
                    0.0f,
                    (float) (z - (height / 2.0) + (args.m_perturb_position ? p_dis(e) : 0.0))
            };
            auto theta = (float) (0.0 + (args.m_perturb_orientation ? r_dis(e) : 0.0));
            Eigen::Vector3f tan{
                    cosf(theta),
                    0.0,
                    sinf(theta)
            };

            auto surfel = std::make_shared<Surfel>(
                    Surfel("s_" + std::to_string(x) + "_" + std::to_string(z),
                           {
                                   {
                                           {x, z, 0},
                                           10.0f,
                                           Eigen::Matrix3f::Identity(),
                                           norm,
                                           pos,
                                   }
                           },
                           tan,
                           {0, 0}));
            auto node = graph->add_node(surfel);
            nodes[z * width + x] = node;
        }
    }
    connectNodes(graph, nodes, width, height);
    return graph;
}


void addSurfel(const std::string &name,
               const Eigen::Vector3f &pos,
               const Eigen::Vector3f &norm,
               const Eigen::Vector3f &tan,
               const Eigen::Matrix3f &transform,
               SurfelGraphPtr &graph,
               SurfelGraphNodePtr *nodes,
               unsigned int index
) {
    auto surfel = std::make_shared<Surfel>(
            Surfel(name,
                   {
                           {
                                   {0, 0, 0},
                                   0.0f,
                                   transform,
                                   norm,
                                   pos,
                           }
                   },
                   tan,
                   {0, 0}));
    auto node = graph->add_node(surfel);
    nodes[index] = node;
}

void addSurfel(const std::string &name,
               const Eigen::Vector3f &pos,
               const Eigen::Vector3f &norm,
               const Eigen::Vector3f &tan,
               SurfelGraphPtr &graph,
               SurfelGraphNodePtr *nodes,
               unsigned int index
) {
    // Transform Y axis into normal
    auto c = Eigen::Vector3f{0, 1, 0}.dot(norm);
    Eigen::Matrix3f transform;
    if (c == 1) {
        transform = Eigen::Matrix3f::Identity();
    } else if (c == -1) {
        transform << 1, 0, 0, 0, -1, 0, 0, 0, 1;
    } else {
        auto v = Eigen::Vector3f{0, 1, 0}.cross(norm);
        auto s = v.norm();
        auto skew = skew_symmetrix_matrix_for(v);
        transform = Eigen::Matrix3f::Identity() + skew + ((skew * skew) * ((1.0 - c) / (s * s)));
    }
    addSurfel(name, pos, norm, tan, transform, graph, nodes, index);
}


/*
*
 */
SurfelGraphPtr generate_sphere(const Args &args) {
    const auto radius = args.m_radius;
    const auto segments = args.m_segments;
    const auto rings = args.m_rings;

    spdlog::info("Generating sphere rings: {}, segments: {}, radius: {:.3f}",
                 rings, segments, radius);


    SurfelGraphPtr graph = std::make_shared<SurfelGraph>();
    SurfelGraphNodePtr nodes[(rings - 1) * segments + 2];

    std::default_random_engine e(123);
    // -/+ 5 degree perturbation
    std::uniform_real_distribution<> p_dis(-M_PI / 36.0f, M_PI / 36.0f);
    std::uniform_real_distribution<> r_dis(-M_PI_4, M_PI_4);
    std::uniform_real_distribution<> o_dis(-M_PI_2, M_PI_2);

    // Vertical angle
    const auto delta_theta = (float) (M_PI / rings);
    // Rot around Y axis
    const auto delta_phi = (float) (2.0 * M_PI / segments);

    int node_index = 0;
    for (unsigned int ring = 1; ring <= rings - 1; ++ring) {
        for (unsigned int seg = 0; seg < segments; ++seg) {

            auto theta = delta_theta * (float) ring;
            auto phi = delta_phi * (float) seg;

            if (args.m_perturb_position) {
                theta += p_dis(e);
                phi += p_dis(e);
            }

            const auto sx = radius * std::sinf(theta) * std::cosf(phi);
            const auto sy = radius * std::sinf(theta) * std::sinf(phi);
            const auto sz = radius * std::cosf(theta);

            Eigen::Vector3f pos{sx, sy, sz};
            const auto norm = pos.normalized();

            auto psi = 0.0f;
            if (args.m_perturb_orientation) {
                psi = o_dis(e);
            }
            const auto tan = Eigen::Vector3f{
                    std::cosf(psi),
                    std::sinf(psi),
                    0,
            };

            auto name = "s_" + std::to_string(ring) + "_" + std::to_string(seg);
            addSurfel(name, pos, norm, tan, graph, nodes, node_index);
            node_index++;
        }
    }
    auto psi = args.m_perturb_orientation
               ? r_dis(e)
               : 0.0f;
    addSurfel("s_top", {0, 0, radius}, {0, 0, 1},
              {
                      std::cosf(psi),
                      std::sinf(psi),
                      0,
              },
              graph, nodes, node_index);
    node_index++;
    addSurfel("s_bot", {0, 0, -radius}, {0, 0, -1},
              {
                      std::cosf(psi),
                      std::sinf(psi),
                      0,
              },
              graph,
              nodes, node_index);

    for (unsigned int ring = 0; ring < rings - 1; ++ring) {
        for (unsigned int seg = 0; seg < segments; ++seg) {
            auto from_index = ring * segments + seg;
            // Can we connect up left?
            if (ring > 0) {
                if (seg > 0) {
                    graph->add_edge(nodes[from_index], nodes[from_index - segments - 1], SurfelGraphEdge{1});
                } else {
                    graph->add_edge(nodes[from_index], nodes[from_index - 1], SurfelGraphEdge{1});
                }

                // Up?
                graph->add_edge(nodes[from_index], nodes[from_index - segments], SurfelGraphEdge{1});

                // Up right ?
                if (seg < segments - 1) {
                    graph->add_edge(nodes[from_index], nodes[from_index - segments + 1], SurfelGraphEdge{1});
                } else {
                    graph->add_edge(nodes[from_index], nodes[from_index - (2 * segments) + 1], SurfelGraphEdge{1});
                }
            }

            // Left
            if (seg > 0) {
                graph->add_edge(nodes[from_index], nodes[from_index - 1], SurfelGraphEdge{1});
            } else {
                graph->add_edge(nodes[from_index], nodes[from_index + segments - 1], SurfelGraphEdge{1});
            }

            // Right?
            if (seg < segments - 1) {
                graph->add_edge(nodes[from_index], nodes[from_index + 1], SurfelGraphEdge{1});
            } else {
                graph->add_edge(nodes[from_index], nodes[from_index - segments + 1], SurfelGraphEdge{1});
            }

            // Down left
            if (ring < rings - 2) {
                if (seg > 0) {
                    graph->add_edge(nodes[from_index], nodes[from_index + segments - 1], SurfelGraphEdge{1});
                } else {
                    graph->add_edge(nodes[from_index], nodes[from_index + (2 * segments) - 1], SurfelGraphEdge{1});
                }

                // Down
                graph->add_edge(nodes[from_index], nodes[from_index + segments], SurfelGraphEdge{1});

                // Down right
                if (seg < segments - 1) {
                    graph->add_edge(nodes[from_index], nodes[from_index + segments + 1], SurfelGraphEdge{1});
                } else {
                    graph->add_edge(nodes[from_index], nodes[from_index + 1], SurfelGraphEdge{1});
                }
            }
        }
    }

    // Connect top
    for (unsigned int seg = 0; seg < segments; ++seg) {
        graph->add_edge(nodes[(rings - 1) * segments], nodes[seg], SurfelGraphEdge{1});
    }
    // Connect bottom
    for (unsigned int seg = 0; seg < segments; ++seg) {
        graph->add_edge(nodes[(rings - 1) * segments + 1], nodes[((rings - 2) * segments) + seg], SurfelGraphEdge{1});
    }

    return graph;
}

Args parseArguments(int argc, char *argv[]) {
    TCLAP::CmdLine cmd("Generate sample data for RoSy smoothing", ' ', "0.9", false);

    TCLAP::ValueArg<std::string> type("t",
                                      "type",
                                      "Planar or Spherical",
                                      true,
                                      "Planar",
                                      "string", cmd);
    TCLAP::ValueArg<unsigned int> width("w",
                                        "width",
                                        "Width of planar and sinusoidal meshes",
                                        false,
                                        DEFAULT_WIDTH,
                                        "int", cmd);
    TCLAP::ValueArg<unsigned int> height("h",
                                         "height",
                                         "Height of planar and sinusoidal meshes",
                                         false,
                                         DEFAULT_HEIGHT,
                                         "int", cmd);

    TCLAP::ValueArg<unsigned int> segments("s",
                                           "segments",
                                           "Number of segments in sphere",
                                           false,
                                           9,
                                           "int", cmd);

    TCLAP::ValueArg<unsigned int> rings("g",
                                        "rings",
                                        "Number of rings in sphere",
                                        false,
                                        9,
                                        "int", cmd);

    TCLAP::ValueArg<float> radius("r",
                                  "radius",
                                  "Radius of sphere",
                                  false,
                                  5.0,
                                  "float", cmd);

    TCLAP::UnlabeledValueArg<std::string> outputFile("out_file_name",
                                                     "Data will be saved here",
                                                     true,
                                                     "",
                                                     "string");
    cmd.add(outputFile);

    TCLAP::SwitchArg perturbPosition("p", "perturb_position", "Move positional fied around", cmd, false);
    TCLAP::SwitchArg perturbOrientation("o", "perturb_orientation", "Move orientation fied around", cmd, false);
    // Parse the argv array.
    cmd.parse(argc, argv);

    Args args{};
    args.m_height = height.getValue();
    args.m_width = width.getValue();
    args.m_perturb_position = perturbPosition.getValue();
    args.m_perturb_orientation = perturbOrientation.getValue();
    args.m_output_file_name = outputFile.getValue();
    args.m_type = (type.getValue() == "planar" ? Args::PLANAR : Args::SPHERICAL);
    args.m_radius = radius.getValue();
    args.m_rings = rings.getValue();
    args.m_segments = segments.getValue();

    return args;
}

int main(int argc, char *argv[]) {
    const auto args = parseArguments(argc, argv);

    SurfelGraphPtr graph;

    switch (args.m_type) {
        case Args::PLANAR:
            graph = generate_plane(args);
            break;

        case Args::SPHERICAL:
            graph = generate_sphere(args);
            break;
    }
    save_surfel_graph_to_file(args.m_output_file_name, graph);
}