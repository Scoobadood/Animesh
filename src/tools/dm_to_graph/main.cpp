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

bool plausible_neighbours( //
    const SurfelGraphNodePtr &node1, //
    const SurfelGraphNodePtr &node2, //
    unsigned int num_frames, //
    float nbr_threshold //
    ) {
  // Compute the distance between these nodes to determine if they are neighbours
  unsigned int shared_frames = 0;
  for (unsigned int frameIdx = 0; frameIdx < num_frames; ++frameIdx) {
    if (!node1->data()->is_in_frame(frameIdx)) {
      continue;
    }
    if (!node2->data()->is_in_frame(frameIdx)) {
      continue;
    }

    Eigen::Vector3f v1, n1, t1, v2, n2, t2;
    node1->data()->get_vertex_tangent_normal_for_frame(frameIdx, v1, t1, n1);
    node2->data()->get_vertex_tangent_normal_for_frame(frameIdx, v2, t2, n2);

    if ((v1 - v2).norm() > nbr_threshold) {
      return false;
    }

    ++shared_frames;
  }
  if (shared_frames == 1) {
    return false;
  }

  return true;
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
        if( dx == 0 && dy == 0 ) {
          continue;
        }
        const auto &n = pif_to_graph_node.find({pif.pixel.x + dx, pif.pixel.y + dy, pif.frame});
        // Not an actual node
        if (n == pif_to_graph_node.end()) {
          continue;
        }
        // Don't double count.
        if( potential_neighbours.count(n->second)  > 0 ) {
          continue;
        }
        potential_neighbours.emplace(n->second);
      }
    }
  }
  return vector<SurfelGraphNodePtr>{begin(potential_neighbours),end(potential_neighbours)};
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
    for (const auto &potential_neighbour_node: potential_neighbour_nodes) {
      if (plausible_neighbours(node, potential_neighbour_node, num_frames, nbr_threshold)) {
        try {
          graph->add_edge(node, potential_neighbour_node, SurfelGraphEdge{1});
        } catch (std::runtime_error const &err) {
        }
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
  auto point_cloud_template = properties.getProperty("d2g-point-cloud-file-template");
  auto eight_connected = properties.getBooleanProperty("d2g-eight-connected");
  auto source_directory = properties.getProperty("d2g-source-directory");
  auto nbr_threshold = properties.getFloatProperty("d2g-max-neighbour-distance");


  // Load all the data
  unsigned int num_frames = 0;
  auto pixel_by_frame = load_pifs(source_directory, pif_regex, num_frames);
  auto normals_by_frame = load_normals(normal_file_template, num_frames, level);
  string pattern = file_name_from_template_and_level("pointcloud_l%02d_f([0-9]{2}).txt", level);
  auto vertices_by_frame = load_vec3s_from_directory(source_directory, pattern);

  // Load paths
  auto paths = load_paths("paths.txt");

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

  save_surfel_graph_to_file("sfl_0" + to_string(level) + ".bin", static_cast<const SurfelGraphPtr>(graph));

  return 0;
}