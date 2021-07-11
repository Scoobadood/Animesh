#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <random>
#include <cstdlib>
#include <Eigen/Geometry>
#include <Geom/Geom.h>
#include <tclap/CmdLine.h>
#include <Surfel/SurfelBuilder.h>

std::default_random_engine defaultRandomEngine{123};

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
const int DEFAULT_DEPTH = 10;
struct Args {
    enum type {
        PLANAR,
        SPHERICAL,
        CYLINDRICAL
    };

    type m_type;
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_depth;
    unsigned int m_rings;
    unsigned int m_segments;
    float m_radius;
    bool m_perturb_orientation;
    bool m_perturb_position;
    bool m_open_cylinder;
    bool m_double_neighbours;
    bool m_spherical_poles;
    std::string m_output_file_name;
};


void
connect(SurfelGraphPtr &graph,
        const SurfelGraphNodePtr &from_node,
        const SurfelGraphNodePtr &to_node) {
    graph->add_edge(from_node, to_node, SurfelGraphEdge{1.0f});
    spdlog::info("Connected {} to {}", from_node->data()->id(), to_node->data()->id());
}

/*
 * Each node in the graph is connected to its 8 surrounding neighbours.
 * We rely on the fact that the graph is not directed so only create edges
 * from (x,y) to (x-1, y-1), (x, y-1), (x+1, y-1) and (x-1,y).
 *      2 2 2 2 2
 *      2 1 1 1 2
 *      2 1 x
 */
void connectNodes(SurfelGraphPtr &graph,
                  SurfelGraphNodePtr *nodes,
                  unsigned int width,
                  unsigned int height,
                  bool double_neighbours = false) {
    for (auto z = 0; z < height; z++) {
        for (auto x = 0; x < width; x++) {
            auto nodeIndex = z * width + x;
            const auto &from_node = nodes[nodeIndex];

            if (double_neighbours) {
                if (z > 0) {
                    if (x > 1) {
                        // (x-2,y-1)
                        const auto &to_node = nodes[nodeIndex - width - 2];
                        connect(graph, from_node, to_node);
                    }
                    if (x < width - 2) {
                        // (x+2,y-1)
                        const auto &to_node = nodes[nodeIndex - width + 2];
                        connect(graph, from_node, to_node);
                    }
                }
                if (z > 1) {
                    if (x > 1) {
                        // (x-2,y-2)
                        const auto &to_node = nodes[nodeIndex - width - width - 2];
                        connect(graph, from_node, to_node);
                    }
                    if (x > 0) {
                        // (x-1,y-2)
                        const auto &to_node = nodes[nodeIndex - width - width - 1];
                        connect(graph, from_node, to_node);
                    }
                    // (x,y-2)
                    const auto &to_node = nodes[nodeIndex - width - width];
                    connect(graph, from_node, to_node);

                    if (x < width - 1) {
                        // (x+1,y-2)
                        const auto &to_node = nodes[nodeIndex - width - width + 1];
                        connect(graph, from_node, to_node);
                    }
                    if (x < width - 2) {
                        // (x+2,y-2)
                        const auto &to_node = nodes[nodeIndex - width - width + 2];
                        connect(graph, from_node, to_node);
                    }
                }
                if (x > 1) {
                    // (x-2,y)
                    const auto &to_node = nodes[nodeIndex - 2];
                    connect(graph, from_node, to_node);
                }
            }

            if (z > 0) {
                if (x > 0) {
                    // (x-1, z-1)
                    const auto &to_node = nodes[nodeIndex - width - 1];
                    connect(graph, from_node, to_node);
                }
                // (x,z-1)
                const auto &to_node = nodes[nodeIndex - width];
                connect(graph, from_node, to_node);
                if (x < width - 1) {
                    // (x+1,z-1)
                    const auto &to_node = nodes[nodeIndex - width + 1];
                    connect(graph, from_node, to_node);
                }
            }
            if (x > 0) {
                // (x-1,y)
                const auto &to_node = nodes[nodeIndex - 1];
                connect(graph, from_node, to_node);
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


    std::uniform_real_distribution<float> p_dis{-0.4, 0.4};

    for (unsigned int x = 0; x < width; x++) {
        for (unsigned int z = 0; z < height; z++) {
            auto *sb = new SurfelBuilder(defaultRandomEngine);
            sb = sb->with_id("s_" + std::to_string(x) + "_" + std::to_string(z));

            Eigen::Vector3f norm{0, 1, 0};
            Eigen::Vector3f pos{
                    (float) (x - ((width - 1) / 2.0) + (args.m_perturb_position ? p_dis(defaultRandomEngine) : 0.0)),
                    0.0f,
                    (float) (z - ((height - 1) / 2.0) + (args.m_perturb_position ? p_dis(defaultRandomEngine) : 0.0))
            };
            pos = pos * 0.1;
            sb->with_frame({x, z, 0},
                           10.0f,
                           Eigen::Matrix3f::Identity(),
                           norm,
                           pos
            );
            if (!args.m_perturb_orientation) {
                sb->with_tangent({1.0, 0.0, 0.0});
            }

            auto surfel = std::make_shared<Surfel>(sb->build());
            auto node = graph->add_node(surfel);
            nodes[z * width + x] = node;
            delete sb;
        }
    }
    connectNodes(graph, nodes, width, height);
    return graph;
}

void addSurfel(const std::string &name,
               const Eigen::Vector3f &pos,
               const Eigen::Vector3f &norm,
               SurfelGraphPtr &graph,
               SurfelGraphNodePtr *nodes,
               unsigned int index
) {
    auto surfel_builder = new SurfelBuilder(defaultRandomEngine);
    auto surfel = std::make_shared<Surfel>(
            surfel_builder
                    ->with_id(name)
                    ->with_frame(
                            {0, 0, 0},
                            0.0f,
                            norm,
                            pos
                    )
                    ->build());
    auto node = graph->add_node(surfel);
    nodes[index] = node;
    delete surfel_builder;
}

void addSurfel(const std::string &name,
               const Eigen::Vector3f &pos,
               const Eigen::Vector3f &norm,
               const Eigen::Vector3f &tan,
               SurfelGraphPtr &graph,
               SurfelGraphNodePtr *nodes,
               unsigned int index
) {
    auto surfel_builder = new SurfelBuilder(defaultRandomEngine);
    auto surfel = std::make_shared<Surfel>(
            surfel_builder
                    ->with_id(name)
                    ->with_tangent(tan)
                    ->with_frame(
                            {0, 0, 0},
                            0.0f,
                            norm,
                            pos
                    )
                    ->build());
    auto node = graph->add_node(surfel);
    nodes[index] = node;
    delete surfel_builder;
}

/*
*
 */
SurfelGraphPtr generate_sphere_from_cube(const Args &args) {
    const auto width = args.m_width;
    const auto height = args.m_height;
    const auto depth = args.m_depth;

    spdlog::info("Generating cube sphere width: {}, height: {}, depth: {}",
                 width, height, depth);

    SurfelGraphPtr graph = std::make_shared<SurfelGraph>();

    const auto w2 = width / 2.0f;
    const auto h2 = height / 2.0f;
    const auto d2 = depth / 2.0f;

    const auto radius = sqrtf(w2 * w2 + h2 * h2 + d2 * d2);
    // Generate Top and Bottom face nodes
    {
        SurfelGraphNodePtr nodes[(width - 1) * (depth - 1) * 2];
        int node_index = 0;
        for (auto vy = -h2; vy <= h2; vy += h2) {
            for (auto z = 0; z < depth - 1; ++z) {
                for (auto x = 0; x < width - 1; ++x) {
                    auto vx = x - w2;
                    auto vz = z - d2;
                    Eigen::Vector3f vec{vx, vy, vz};
                    auto norm = vec.normalized();
                    Eigen::Vector3f node = norm * radius;
                    auto name = "s_" + std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);
                    addSurfel(name, node, norm, graph, nodes, node_index);

                    long from_node_index = node_index;
                    long to_node_index;

                    if (x > 0) {
                        to_node_index = from_node_index - 1;
                        graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
                    }
                    if (z > 0) {
                        if (x > 0) {
                            to_node_index = from_node_index - width - 1;
                            graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
                        }
                        to_node_index = from_node_index - width;
                        graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
                        if (x < width - 2) {
                            to_node_index = from_node_index - width + 1;
                            graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
                        }
                    }
                }
                node_index++;
            }
        }
    }

//    {
//        // Front and Back
//        SurfelGraphNodePtr nodes[(width + 1) * (height + 1) * 2];
//        int node_index = 0;
//
//        for (auto y = 0; y < height; ++y) {
//            for (auto x = 0; x < width; ++x) {
//                auto vx = x - w2;
//                auto vy = y - h2;
//                auto vz = 0 - d2;
//                Eigen::Vector3f vec{vx, vy, vz};
//                auto norm = vec.normalized();
//                Eigen::Vector3f node = norm * radius;
//                auto name = "s_" + std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);
//                addSurfel(name, node, norm, graph, nodes, node_index++);
//
//                vz = d2;
//                vec.z() = vz;
//                norm = vec.normalized();
//                node = norm * radius;
//                name = "s_" + std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);
//                addSurfel(name, node, norm, graph, nodes, node_index++);
//
//
//                long from_node_index = node_index - 1;
//                long to_node_index;
//
//                if (x > 0) {
//                    to_node_index = from_node_index - 2;
//                    graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                    graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                }
//                if (y > 0) {
//                    if (x > 0) {
//                        to_node_index = from_node_index - width - 2;
//                        graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                        graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                    }
//                    to_node_index = from_node_index - width;
//                    graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                    graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                    if (x < width - 1) {
//                        to_node_index = from_node_index - width + 2;
//                        graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                        graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                    }
//                }
//            }
//        }
//    }
//
//    {
//        // Left and Right
//        SurfelGraphNodePtr nodes[(height + 1) * (depth + 1) * 2];
//        int node_index = 0;
//
//        for (auto z = 0; z < depth; ++z) {
//            for (auto y = 0; y < height; ++y) {
//                auto vx = 0 - w2;
//                auto vy = y - h2;
//                auto vz = z - d2;
//                Eigen::Vector3f vec{vx, vy, vz};
//                auto norm = vec.normalized();
//                Eigen::Vector3f node = norm * radius;
//                auto name = "s_" + std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);
//                addSurfel(name, node, norm, graph, nodes, node_index++);
//
//                vx = w2;
//                vec.x() = vx;
//                norm = vec.normalized();
//                node = norm * radius;
//                name = "s_" + std::to_string(vx) + "_" + std::to_string(vy) + "_" + std::to_string(vz);
//                addSurfel(name, node, norm, graph, nodes, node_index++);
//
//                long from_node_index = node_index - 1;
//                long to_node_index;
//                if (y > 0) {
//                    to_node_index = from_node_index - 2;
//                    graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                    graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                }
//                if (z > 0) {
//                    if (y > 0) {
//                        to_node_index = from_node_index - height - 2;
//                        graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                        graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                    }
//                    to_node_index = from_node_index - height;
//                    graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                    graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                    if (y < height - 1) {
//                        to_node_index = from_node_index - height + 2;
//                        graph->add_edge(nodes[from_node_index], nodes[to_node_index], SurfelGraphEdge{1});
//                        graph->add_edge(nodes[from_node_index - 1], nodes[to_node_index - 1], SurfelGraphEdge{1});
//                    }
//                }
//            }
//        }
//    }

    return graph;
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
                theta += p_dis(defaultRandomEngine);
                phi += p_dis(defaultRandomEngine);
            }

            const auto sx = radius * std::sinf(theta) * std::cosf(phi);
            const auto sy = radius * std::sinf(theta) * std::sinf(phi);
            const auto sz = radius * std::cosf(theta);

            Eigen::Vector3f pos{sx, sy, sz};
            const auto norm = pos.normalized();

            if (args.m_perturb_orientation) {
                auto name = "s_" + std::to_string(ring) + "_" + std::to_string(seg);
                addSurfel(name, pos, norm, graph, nodes, node_index);
            } else {
                Eigen::Vector3f tan = norm.cross(Eigen::Vector3f{0, 0, 1});
                addSurfel("s_" + std::to_string(ring) + "_" + std::to_string(seg),
                          pos, norm, tan, graph, nodes, node_index);
            }
            node_index++;
        }
    }
    if (args.m_spherical_poles) {
        addSurfel("s_top", {0, 0, radius}, {0, 0, 1},
                  graph,
                  nodes, node_index);
        node_index++;
        addSurfel("s_bot", {0, 0, -radius}, {0, 0, -1},
                  graph,
                  nodes, node_index);
    }
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

    if (args.m_spherical_poles) {
        // Connect top
        for (unsigned int seg = 0; seg < segments; ++seg) {
            graph->add_edge(nodes[(rings - 1) * segments], nodes[seg], SurfelGraphEdge{1});
        }
        // Connect bottom
        for (unsigned int seg = 0; seg < segments; ++seg) {
            graph->add_edge(nodes[(rings - 1) * segments + 1], nodes[((rings - 2) * segments) + seg],
                            SurfelGraphEdge{1});
        }
    }
    return graph;
}

Args parseArguments(int argc, char *argv[]) {
    TCLAP::CmdLine cmd("Generate sample data for RoSy smoothing", ' ', "0.9", false);

    TCLAP::ValueArg<std::string> type("t",
                                      "type",
                                      "Planar, Cylindrical or Spherical",
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
    TCLAP::ValueArg<unsigned int> depth("d",
                                        "depth",
                                        "Depth of cube",
                                        false,
                                        DEFAULT_DEPTH,
                                        "int", cmd);

    TCLAP::ValueArg<unsigned int> segments("s",
                                           "segments",
                                           "Number of segments",
                                           false,
                                           9,
                                           "int", cmd);

    TCLAP::ValueArg<unsigned int> rings("g",
                                        "rings",
                                        "Number of rings",
                                        false,
                                        9,
                                        "int", cmd);

    TCLAP::ValueArg<float> radius("r",
                                  "radius",
                                  "Radius",
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
    TCLAP::SwitchArg cut("x", "cut", "If set, do not close cylinder", cmd, false);
    TCLAP::SwitchArg dbl("2", "double", "If set, double the neighbours", cmd, false);
    TCLAP::SwitchArg poles("P", "exclude_poles", "If set, don't render poles of sphere", cmd, false);

    // Parse the argv array.
    cmd.parse(argc, argv);

    Args args{};
    args.m_height = height.getValue();
    args.m_width = width.getValue();
    args.m_depth = depth.getValue();
    args.m_spherical_poles = poles.getValue();
    args.m_perturb_position = perturbPosition.getValue();
    args.m_perturb_orientation = perturbOrientation.getValue();
    args.m_open_cylinder = cut.getValue();
    args.m_double_neighbours = dbl.getValue();
    args.m_output_file_name = outputFile.getValue();
    args.m_type = (type.getValue() == "planar")
                  ? Args::PLANAR
                  : (type.getValue() == "cylindrical")
                    ? Args::CYLINDRICAL
                    : Args::SPHERICAL;
    args.m_radius = radius.getValue();
    args.m_rings = rings.getValue();
    args.m_segments = segments.getValue();

    return args;
}

/*
 * Build a cylinder, circular cross-section in the XZ plane and
 * the height is along the Y-axis.
 */
SurfelGraphPtr
generate_cylinder(const Args &args) {
    const auto segments = args.m_segments;
    const auto rings = args.m_rings;
    const auto radius = args.m_radius;

    const auto perturb_position = args.m_perturb_position;

    spdlog::info("Generating cylinder. Rings: {}, segments {}, radius: {} ", rings, segments, radius);

    SurfelGraphPtr graph = std::make_shared<SurfelGraph>();
    SurfelGraphNodePtr nodes[(rings + 1) * segments];

    unsigned int node_index = 0;
    const auto dTheta = (float) (2.0f * M_PI) / (float) segments;

    std::uniform_real_distribution<> z_dis(-0.2f, 0.2f);
    std::uniform_real_distribution<> xy_dis(-dTheta / 4.0f, dTheta / 4.0f);
    for (unsigned int r = 0; r <= rings; ++r) {
        for (unsigned int s = 0; s < segments; ++s) {
            const auto theta = dTheta * (float) s;

            const auto positional_perturbation_xy = (perturb_position)
                                                    ? xy_dis(defaultRandomEngine)
                                                    : 0.0f;
            const auto positional_perturbation_z = (perturb_position)
                                                   ? z_dis(defaultRandomEngine)
                                                   : 0.0f;

            Eigen::Vector3f pos{
                    (float) radius * std::cosf(theta + positional_perturbation_xy),  //
                    (float) radius * std::sinf(theta + positional_perturbation_xy),
                    (float) r - (rings / 2.0f) + positional_perturbation_z
            };
            Eigen::Vector3f norm = Eigen::Vector3f{pos[0], pos[1], 0}.normalized();
            // If we have orientation perturbation, allow surfels to generate their own tans
            if (args.m_perturb_orientation) {
                addSurfel("s_" + std::to_string(r) + "_" + std::to_string(s),
                          pos, norm, graph, nodes, node_index);
            } else {
                Eigen::Vector3f tan{0, 0, 1};
                addSurfel("s_" + std::to_string(r) + "_" + std::to_string(s),
                          pos, norm, tan, graph, nodes, node_index);
            }
            node_index++;
        }
    }
    connectNodes(graph, nodes, segments, rings + 1, args.m_double_neighbours);
    // Also connect the right hand edge to the left
    if (!args.m_open_cylinder) {
        for (int r = 0; r <= rings; ++r) {
            unsigned int nodeIndex = ((r + 1) * segments) - 1;
            const auto &from_node = nodes[nodeIndex];
            // Always connect to first node on this level
            const auto &to_node = nodes[nodeIndex - (segments - 1)];
            connect(graph, from_node, to_node);
            if (r > 1) {
                // Connect down where possible
                const auto &to_node = nodes[nodeIndex - segments - (segments - 1)];
                connect(graph, from_node, to_node);
            }
            if (r < rings) {
                // Connect up where available
                const auto &to_node = nodes[nodeIndex + segments - (segments - 1)];
                connect(graph, from_node, to_node);
            }
        }
    }
    return graph;
}

int main(int argc, char *argv[]) {
    const auto args = parseArguments(argc, argv);

    SurfelGraphPtr graph;

    switch (args.m_type) {
        case Args::PLANAR:
            graph = generate_plane(args);
            break;

        case Args::SPHERICAL:
            graph = generate_sphere_from_cube(args);
            //graph = generate_sphere(args);
            break;

        case Args::CYLINDRICAL:
            graph = generate_cylinder(args);
            break;
    }
    save_surfel_graph_to_file(args.m_output_file_name, graph);
}