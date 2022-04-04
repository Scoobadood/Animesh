//
// Created by Dave Durbin on 11/12/21.
//

#include <CommonUtilities/split.h>
#include <FileUtils/FileUtils.h>
#include <Properties/Properties.h>
#include <Surfel/PixelInFrame.h>
#include <string>
#include <regex>
#include <Surfel/SurfelGraph.h>
#include <Surfel/SurfelBuilder.h>
#include <Surfel/Surfel_IO.h>
#include <Tools/tools.h>

// frame/idx
std::vector<std::vector<std::pair<unsigned int, unsigned int>>> load_paths(const std::string &filename) {
  using namespace std;


  vector<vector<pair<unsigned int, unsigned int>>> paths;

  process_file_by_lines(filename, [&paths](const std::string &line) {
    using namespace std;

    vector<pair<unsigned int, unsigned int>> path;

    vector<string> path_entries = split(line, ',');
    string rs = R"(\w*\(([0-9]*) ([0-9]*)\)\w*)";
    regex entry_r{rs};
    smatch matches;

    for (const auto &entry: path_entries) {

      if (!regex_search(entry, matches, entry_r)) {
        throw runtime_error("Invalid path entry  " + entry);
      }

      // 0 is the whole string, 1 is fr, 2 is idx
      unsigned int frameIdx = stoi(matches[1].str());
      unsigned int idx = stoi(matches[2].str());
      path.emplace_back(frameIdx, idx);
    }
    paths.push_back(path);
  });
  return paths;
}

std::string
file_name_from_template_level_and_frame(const std::string &file_name_template, unsigned int level, unsigned int frame) {
  // We expect 2x %2dL
  size_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), level, frame);
  char file_name[bufsz + 1];
  snprintf(file_name, bufsz + 1, file_name_template.c_str(), level, frame);
  return file_name;
}

std::string
file_name_from_template_and_level(const std::string &file_name_template, unsigned int level) {
  // We expect 1x %2dL
  ssize_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), level);
  char file_name[bufsz + 1];
  snprintf(file_name, bufsz + 1, file_name_template.c_str(), level);
  return file_name;
}

void
cull_unreasonable_neighbours(
    const SurfelGraphNodePtr &node,
    std::vector<SurfelGraphNodePtr> &potential_neighbour_nodes) {

  auto &this_surfel = node->data();

  bool should_log = (potential_neighbour_nodes.size() >= 8);

  if (should_log) {
    spdlog::info(
        "Found a point with {} potential neighbours. Checking for culling of outliers.",
        potential_neighbour_nodes.size());
  }

  std::set<SurfelGraphNodePtr> cullable_neighbours;
  for (const auto &neighbour: potential_neighbour_nodes) {
    auto &that_surfel = neighbour->data();
    int this_surfel_frame_idx = 0;
    int that_surfel_frame_idx = 0;
    std::vector<std::pair<int, float>> frame_to_dist;

    while ((this_surfel_frame_idx < this_surfel->num_frames()) &&
        (that_surfel_frame_idx < that_surfel->num_frames())) {
      auto this_surfel_frame = this_surfel->frames()[this_surfel_frame_idx];
      auto that_surfel_frame = that_surfel->frames()[that_surfel_frame_idx];
      if (this_surfel_frame < that_surfel_frame) {
        ++this_surfel_frame_idx;
        continue;
      }
      if (this_surfel_frame > that_surfel_frame) {
        ++that_surfel_frame_idx;
        continue;
      }
      // Surfels coexist in this frame
      float delta = (this_surfel->frame_data()[this_surfel_frame_idx].position
          - that_surfel->frame_data()[this_surfel_frame_idx].position).norm();
      frame_to_dist.emplace_back(this_surfel_frame, delta);
      ++this_surfel_frame_idx;
      ++that_surfel_frame_idx;
    }

    // OK we have a bunch of frames and associated distances
    if (frame_to_dist.size() == 1) {
      if (should_log) {
        // Only neighbours in one frame - not really trustworthy.
        spdlog::info("Point is only a neighbour on the basis of a single frame. Culling.");
      }
      cullable_neighbours.emplace(neighbour);
      continue;
    } else {
      if (should_log) {
        spdlog::info(
            "Neighbour is apparently present in {} frames.",
            frame_to_dist.size());
      }
    }


    // Sort frame_to_dist by dist.
    std::sort(frame_to_dist.begin(),
              frame_to_dist.end(),
              [](const std::pair<int, float> &p1, const std::pair<int, float> &p2) {
                return p1.second < p2.second;
              });

    // These values...
    if (should_log) {
      for (const auto &x: frame_to_dist) {
        spdlog::info(" [{}, {}] ", x.first, x.second);
      }
    }

    // Compute q1 and q3
    float q1, q2, q3;
    size_t q2_index = frame_to_dist.size() / 2;
    size_t q1_index = q2_index / 2;
    size_t q3_index = q2_index + q1_index + ((frame_to_dist.size() % 2 == 1) ? 1 : 0);

    if (frame_to_dist.size() % 2 == 1) {
      q2 = frame_to_dist[q2_index].second;
    } else {
      q2 = (frame_to_dist[q2_index].second + frame_to_dist[q2_index - 1].second) / 2.0f;
    }
    if (q2_index % 2 == 1) {
      q1 = frame_to_dist[q1_index].second;
      q3 = frame_to_dist[q3_index].second;
    } else {
      q1 = (frame_to_dist[q1_index].second + frame_to_dist[q1_index - 1].second) / 2.0f;
      q3 = (frame_to_dist[q3_index].second + frame_to_dist[q3_index - 1].second) / 2.0f;
    }

    // Compute IQR
    auto iqr = q3 - q1;
    // Max value is q3.dist + 1.5 * IQR
    auto max_dist = q3 + (1.0f * iqr);
    // Anything greater than this is an outlier.

    if (should_log) {
      spdlog::info(" q1: {}, q2: {}, q3: {}, IQR: {}, max_dist: {}", q1, q2, q3, iqr, max_dist);
    }

    bool bad_value_present = false;
    for (const auto &frame_dist: frame_to_dist) {
      if (frame_dist.second > max_dist) {
        if (should_log) {
          spdlog::info("   Distance in {} frame {} is an outlier.", frame_dist.second, frame_dist.first);
        }

        bad_value_present = true;
        break;
      }
    }

    if (bad_value_present) {
      cullable_neighbours.emplace(neighbour);
      if (should_log) {
        spdlog::info("   Culling this neighbour.");
      }
    } else {
      if (should_log) {
        spdlog::info("   This neighbour is fine.");
      }
    }
  }

  if (!cullable_neighbours.empty()) {
    if (should_log) {
      spdlog::info(" Found {} cullable neighbours.", cullable_neighbours.size());
    }

    potential_neighbour_nodes.erase(
        std::remove_if(potential_neighbour_nodes.begin(),
                             potential_neighbour_nodes.end(),
                             [&](const SurfelGraphNodePtr &n) {
                               return cullable_neighbours.count(n) > 0;
                             }),
              potential_neighbour_nodes.end()
    );
  }
  if (should_log) {
    spdlog::info(" Removed {} nodes. Now {} left",
                 cullable_neighbours.size(),
                 potential_neighbour_nodes.size());
  }
}

std::vector<SurfelGraphNodePtr>
get_potential_neighbours( //
    const std::map<PixelInFrame, SurfelGraphNodePtr> &pif_to_graph_node, //
    const SurfelGraphNodePtr &node) {
  using namespace std;

  set<SurfelGraphNodePtr> potential_neighbours;

  for (const auto &fd: node->data()->frame_data()) {
    const auto &pif = fd.pixel_in_frame;

    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        // Not my own neighbour
        if (dx == 0 && dy == 0) {
          continue;
        }
        const auto &n = pif_to_graph_node.find({pif.pixel.x + dx, pif.pixel.y + dy, pif.frame});
        // Not an actual node
        if (n == pif_to_graph_node.end()) {
          continue;
        }
        // Don't double count.
        if (potential_neighbours.count(n->second) > 0) {
          continue;
        }
        potential_neighbours.emplace(n->second);
      }
    }
  }
  return vector<SurfelGraphNodePtr>{begin(potential_neighbours), end(potential_neighbours)};
}

void
generate_edges( //
    SurfelGraph *graph, //
    const std::map<PixelInFrame, SurfelGraphNodePtr> &pif_to_graph_node, //
    unsigned int num_frames, //
    float nbr_threshold //
) {
  //    Add neighbours based on PIF data
  for (const auto &node: graph->nodes()) {
    auto potential_neighbour_nodes = get_potential_neighbours(pif_to_graph_node, node);
    cull_unreasonable_neighbours(node, potential_neighbour_nodes);
    for (const auto &neighbour_node: potential_neighbour_nodes) {
      try {
        graph->add_edge(node, neighbour_node, SurfelGraphEdge{1});
      } catch (std::runtime_error const &err) {
      }
    }
  }
}

std::map<unsigned int, std::vector<Eigen::Vector3f>>
load_normals(
    const std::string &normal_file_template,
    unsigned int num_frames,
    unsigned int level
) {
  using namespace std;
  using namespace Eigen;

  map<unsigned int, vector<Vector3f>> normals_by_frame;
  for (int frameIdx = 0; frameIdx < num_frames; ++frameIdx) {
    auto normal_file_name = file_name_from_template_level_and_frame(normal_file_template, level, frameIdx);
    process_file_by_lines(normal_file_name, [&normals_by_frame, &frameIdx](const std::string &line) {
      auto normal = string_to_vec3f(line);
      normals_by_frame[frameIdx].emplace_back(normal);
    });
  }
  return normals_by_frame;
}

unsigned int extract_frame_from_pif_filename(std::string file_name, const std::string &pif_regex) {
  using namespace std;

  const regex file_name_regex(pif_regex);
  smatch matches;
  transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
  if (regex_search(file_name, matches, file_name_regex)) {
    // 0 is the whole string, 1 is the frame
    return stoi(matches[1].str());
  }
  throw runtime_error("pif file name is invalid " + file_name);
}

std::vector<std::vector<Pixel>> load_pifs( //
    const std::string &source_directory, //
    const std::string &pif_regex, //
    unsigned int &num_frames) {
  using namespace std;

  vector<string> pif_files;
  const regex file_name_regex(pif_regex);
  files_in_directory(source_directory, pif_files, [&file_name_regex](string name) {
    using namespace std;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    smatch matches;
    return regex_search(name, matches, file_name_regex);
  });

  if (pif_files.empty()) {
    throw runtime_error("No PIF files found in " + source_directory);
  }

  std::sort(pif_files.begin(), pif_files.end());

  vector<vector<Pixel>> pixel_by_frame;
  num_frames = 0;
  for (const auto &pif_file: pif_files) {
    auto frameIdx = extract_frame_from_pif_filename(pif_file, pif_regex);
    if (frameIdx >= num_frames) {
      num_frames = frameIdx + 1;
      pixel_by_frame.resize(num_frames);
    }

    size_t index = 0;
    process_file_by_lines(pif_file, [&pixel_by_frame, frameIdx, &index](const std::string &line) {
      // Strip ()
      auto values = line.substr(1, line.size() - 2);
      auto coords = split(values, ',');
      unsigned int px = stoi(coords[0]);
      unsigned int py = stoi(coords[1]);
      pixel_by_frame[frameIdx].push_back({px, py});
      ++index;
    });
  }
  return pixel_by_frame;
}

int main(int argc, const char *argv[]) {
  using namespace std;
  using namespace Eigen;

  string properties_file_name = (argc < 2)
                                ? "animesh.properties"
                                : argv[1];
  Properties properties{properties_file_name};

  // load important properties
  auto level = properties.getIntProperty("d2g-level");
  auto pif_regex = properties.getProperty("d2g-pif-regex");
  auto corr_file_template = properties.getProperty("d2g-correspondence-file-template");
  auto normal_file_template = properties.getProperty("d2g-normal-file-template");
  auto point_cloud_regex = properties.getProperty("d2g-point-cloud-regex");
  auto eight_connected = properties.getBooleanProperty("d2g-eight-connected");
  auto source_directory = properties.getProperty("d2g-source-directory");
  auto nbr_threshold = properties.getFloatProperty("d2g-max-neighbour-distance");

  cout << "Running " << argv[0] << " with the following settings:" << endl;
  cout << "  d2g-level                        : " << level << endl;
  cout << "  d2g-pif-regex                    : " << pif_regex << endl;
  cout << "  d2g-correspondence-file-template : " << corr_file_template << endl;
  cout << "  d2g-normal-file-template         : " << normal_file_template << endl;
  cout << "  d2g-point-cloud-regex            : " << point_cloud_regex << endl;
  cout << "  d2g-eight-connected              : " << (eight_connected ? "true" : "false") << endl;
  cout << "  d2g-source-directory             : " << source_directory << endl;
  cout << "  d2g-max-neighbour-distance       : " << nbr_threshold << endl;

  string path_file_name;
  if (properties.hasProperty("d2g-path-template")) {
    path_file_name = file_name_from_template_and_level(properties.getProperty("d2g-path-template"), level);
  } else {
    path_file_name = "paths.txt";
  }
  cout << "Loading paths from : " << path_file_name << endl;

  string surfel_file_name;
  if (properties.hasProperty("d2g-surfel-file-template")) {
    surfel_file_name = file_name_from_template_and_level(properties.getProperty("d2g-surfel-file-template"), level);
  } else {
    surfel_file_name = "surfels.bin";
  }
  cout << "Saving output graph to : " << surfel_file_name << endl;

  // Load all the data
  unsigned int num_frames = 0;
  string pattern = file_name_from_template_and_level(pif_regex, level);
  auto pixel_by_frame = load_pifs(source_directory, pattern, num_frames);

  pattern = file_name_from_template_and_level(point_cloud_regex, level);
  auto vertices_by_frame = load_vec3s_from_directory(source_directory, pattern);

  auto normals_by_frame = load_normals(normal_file_template, num_frames, level);

  // Load paths
  auto paths = load_paths(path_file_name);
  cout << "Found " << paths.size() << " paths." << endl;

  // Use the paths to generate surfels
  map<PixelInFrame, SurfelGraphNodePtr> pif_to_surfel;

  default_random_engine re{123};
  auto sb = new SurfelBuilder(re);
  auto *graph = new SurfelGraph();

  unsigned int surfel_id = 0;
  for (const auto &path: paths) {
    sb->reset();
    string surfel_name = "s_" + to_string(surfel_id);
    sb->with_id(surfel_name);

    for (const auto &path_entry: path) {
      PixelInFrame pif{pixel_by_frame[path_entry.first][path_entry.second], path_entry.first};
      auto &v = vertices_by_frame[path_entry.first][path_entry.second];
      auto &n = normals_by_frame[path_entry.first][path_entry.second];
      sb->with_frame(pif, 0.0f, n, v);
    }

    auto node = graph->add_node(make_shared<Surfel>(sb->build()));
    for (const auto &path_entry: path) {
      PixelInFrame pif{pixel_by_frame[path_entry.first][path_entry.second], path_entry.first};
      pif_to_surfel.emplace(pif, node);
    }

    ++surfel_id;
  }

  // Use adjacency of pixels in DMs to establish neighbourhoods
  generate_edges(graph, pif_to_surfel, num_frames, nbr_threshold);

  save_surfel_graph_to_file(surfel_file_name, static_cast<const SurfelGraphPtr>(graph));

  return 0;
}