//
// Created by Dave Durbin on 11/12/21.
//

#include <CommonUtilities/split.h>
#include <FileUtils/FileUtils.h>
#include <Properties/Properties.h>
#include <Surfel/PixelInFrame.h>
#include <Eigen/Eigen>
#include <string>
#include <regex>
#include <Surfel/SurfelGraph.h>
#include <Surfel/SurfelBuilder.h>
#include <Surfel/Surfel_IO.h>
#include "../../libCorrespondence/include/Correspondence/CorrespondenceIO.h"

std::string
file_name_from_template_level_and_frame(const std::string &file_name_template, unsigned int level, unsigned int frame) {
  // We expect 2x %2dL
  ssize_t bufsz = snprintf(nullptr, 0, file_name_template.c_str(), level, frame);
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

void generate_edges(SurfelGraph * graph, const std::map<PixelInFrame, SurfelGraphNodePtr> &pif_to_graph_node) {
  //    Add neighbours based on PIF data
  for (const auto &node: graph->nodes()) {
    for (const auto &fd: node->data()->frame_data()) {
      const auto &pif = fd.pixel_in_frame;
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          PixelInFrame check{pif.pixel.x + dx, pif.pixel.y + dy, pif.frame};
          const auto &n = pif_to_graph_node.find(check);
          if (n != pif_to_graph_node.end()) {
            try {
              graph->add_edge(node, n->second, SurfelGraphEdge{1});
            } catch (std::runtime_error const &err) {
            }
          }
        }
      }
    }
  }
}

std::vector<std::vector<PixelInFrame>> load_correspondences( //
    const std::string &corr_file_template,
    unsigned int level) {
  using namespace std;

  auto file_name = file_name_from_template_and_level(corr_file_template, level);
  vector<vector<PixelInFrame>> correspondences;
  load_correspondences_from_file(file_name, correspondences);
  return correspondences;
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
};

std::map<unsigned int, std::vector<Eigen::Vector3f>>
load_vertices(
    const std::string &point_cloud_template,
    unsigned int num_frames,
    unsigned int level
) {
  using namespace std;
  using namespace Eigen;

  map<unsigned int, vector<Vector3f>> vertices_by_frame;
  for (int frameIdx = 0; frameIdx < num_frames; ++frameIdx) {
    auto vertex_file_name = file_name_from_template_level_and_frame(point_cloud_template, level, frameIdx);
    process_file_by_lines(vertex_file_name, [&vertices_by_frame, &frameIdx](const std::string &line) {
      auto normal = string_to_vec3f(line);
      vertices_by_frame[frameIdx].emplace_back(normal);
    });
  }
  return vertices_by_frame;
};

std::map<PixelInFrame, std::pair<Eigen::Vector3f, Eigen::Vector3f>> load_vertices_and_normals( //
    const std::map<PixelInFrame, size_t> &map_pif_to_index, //
    const std::string &normal_file_template, //
    const std::string &point_cloud_template, //
    unsigned int num_frames, //
    unsigned int level
) {
  using namespace std;
  using namespace Eigen;

  auto normals_by_frame = load_normals(normal_file_template, num_frames, level);
  auto vertices_by_frame = load_vertices(point_cloud_template, num_frames, level);

  map<PixelInFrame, pair<Vector3f, Vector3f>> pif_to_data;
  for (const auto &pif_frame: map_pif_to_index) {
    auto frame = pif_frame.first.frame;
    auto idx = pif_frame.second;
    auto v = vertices_by_frame[frame][idx];
    auto n = normals_by_frame[frame][idx];
    pif_to_data.emplace(pif_frame.first, make_pair(v, n));
  }
  return pif_to_data;
}

unsigned int extract_frame_from_pif_filename(std::string file_name, const std::string &pif_regex) {
  using namespace std;

  const regex file_name_regex(pif_regex);
  smatch matches;
  transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
  if (regex_search(file_name, matches, file_name_regex)) {
    // 0 is the whole string, 1 is the level, 2 is the frame
    return stoi(matches[2].str());
  }
  throw runtime_error("pif file name is invalid " + file_name);
}

std::map<PixelInFrame, size_t> load_pifs( //
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

  map<PixelInFrame, size_t> pif_to_index;
  num_frames = 0;
  for (const auto &pif_file: pif_files) {
    auto frameIdx = extract_frame_from_pif_filename(pif_file, pif_regex);
    if (frameIdx >= num_frames) {
      num_frames = frameIdx + 1;
    }
    size_t index = 0;
    process_file_by_lines(pif_file, [&pif_to_index, frameIdx, &index](const std::string &line) {
      // Strip ()
      auto values = line.substr(1, line.size() - 2);
      auto coords = split(values, ',');
      unsigned int px = stoi(coords[0]);
      unsigned int py = stoi(coords[1]);
      pif_to_index.emplace(PixelInFrame{px, py, frameIdx}, index);
      ++index;
    });
  }
  return pif_to_index;
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

  // Load all the data
  unsigned int num_frames = 0;
  auto pifs = load_pifs(source_directory, pif_regex, num_frames);

  // Load vertices and normals and map to PIFs
  auto pif_lookup = load_vertices_and_normals(pifs, normal_file_template, point_cloud_template, num_frames, level);

  // Load correspondences
  auto correspondences = load_correspondences(corr_file_template, level);

  // Use the correspondences to generate surfels
  map<PixelInFrame, SurfelGraphNodePtr> pif_to_surfel;
  default_random_engine re{123};
  auto sb = new SurfelBuilder(re);
  auto *graph = new SurfelGraph();

  unsigned int surfel_id = 0;
  for (const auto &correspondence: correspondences) {
    sb->reset();
    string surfel_name = "s_" + to_string(surfel_id);
    sb->with_id(surfel_name);

    for (const auto &pif: correspondence) {
      auto v_and_n = pif_lookup.at(pif);
      sb->with_frame(pif, 0.0f, v_and_n.second, v_and_n.first);
    }

    auto node = graph->add_node(make_shared<Surfel>(sb->build()));
    for (const auto &pif: correspondence) {
      pif_to_surfel.emplace(pif, node);
    }

    ++surfel_id;
  }

  // Use adjacency of pixels in DMs to establish neighbourhoods
  generate_edges(graph, pif_to_surfel);

  save_surfel_graph_to_file("sfl_0" + to_string(level) + ".bin", static_cast<const SurfelGraphPtr>(graph));

  return 0;
}