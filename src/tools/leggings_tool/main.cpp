//
// Created by Dave Durbin (Old) on 26/9/21.
//

#include <tclap/CmdLine.h>
#include <tclap/ArgException.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <GeomFileUtils/ply_io.h>
#include <Surfel/SurfelBuilder.h>
#include <Surfel/SurfelGraph.h>
#include <FileUtils/FileUtils.h>
#include <Surfel/Surfel_IO.h>
#include <GeomFileUtils/io_utils.h>
#include <Eigen/Eigen>
#include <Eigen/Geometry>

struct args {
  std::string ply_file_name;
  std::string anim_file_name;
  std::string surfel_file_name;
  std::vector<int> frames;
};

args parse_args(int argc, char *argv[]) {
  using namespace TCLAP;
  using namespace std;

  try {
    CmdLine cmd("Convert leggings animation data to surfel graph", ' ', "0.1");

    // Define a value argument and add it to the command line.
    // A value arg defines a flag and a type of value that it expects,
    // such as "-n Bishop".
    ValueArg<string> ply_file("p", "ply_file", "The input PLY file name", true, "", "file name", cmd);
    ValueArg<string>
        anim_file("a", "anim_file", "The input text file for animation data", true, "", "file name", cmd);
    ValueArg<string>
        surf_file("s", "surf_file", "The output surfel graph file", false, "surfel_graph.bin", "file name", cmd);
    ValueArg<string> frames
        ("f",
         "frames",
         "Which frames to include in the output. Comma separated list of 1-indexed frame IDs. Defaults to all frames.",
         false,
         "",
         "1,2,3",
         cmd);

    // Parse the argv array.
    cmd.parse(argc, argv);

    args a{};
    a.ply_file_name = ply_file.getValue();
    a.anim_file_name = anim_file.getValue();
    a.surfel_file_name = surf_file.getValue();

    // Convert string of comma-separated ints into vector of int
    if (frames.isSet()) {
      stringstream s_stream{frames.getValue()};
      try {
        while (s_stream.good()) {
          string substr;
          getline(s_stream, substr, ',');
          a.frames.push_back(stoi(substr));
        }
      }
      catch (invalid_argument &e) {
        throw std::runtime_error{"Invalid frame"};
      }
    }

    return a;
  } catch (TCLAP::ArgException &e) { // catch any exceptions
    spdlog::error("error: {} for argument {}.", e.error(), e.argId());
    throw std::runtime_error{"Bad arguments"};
  }
}

typedef struct Face {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void *other_props;       /* other properties */
} Face;

void
read_vertices(PlyFile *ply,
              unsigned int element_id,
              unsigned int num_vertices,
              std::vector<Eigen::Vector3f> &vertices) {
  PlyProperty vert_props[] = { /* list of property information for a vertex */
      {"x", Float32, Float32, 0, 0, 0, 0, 0},
      {"y", Float32, Float32, 4, 0, 0, 0, 0},
      {"z", Float32, Float32, 8, 0, 0, 0, 0},
  };
  setup_property_ply(ply, &vert_props[0]);
  setup_property_ply(ply, &vert_props[1]);
  setup_property_ply(ply, &vert_props[2]);

  Eigen::Vector3f vertex;

  for (unsigned int j = 0; j < num_vertices; j++) {
    get_element_ply(ply, (void *) &vertex.data()[0]);
    vertices.push_back(vertex);
  }
}

void
read_faces(PlyFile *ply,
           unsigned int element_id,
           unsigned int num_faces,
           std::vector<std::vector<std::size_t>> &faces) {
  PlyProperty face_prop{
      "vertex_indices",
      Int32, Int32, offsetof(Face, verts),
      1, Uint8, Uint8, 0};

  setup_property_ply(ply, &face_prop);
  Face f;
  for (int j = 0; j < num_faces; ++j) {
    get_element_ply(ply, (void *) &f);
    std::vector<std::size_t> face;
    for (int k = 0; k < f.nverts; ++k) {
      face.push_back(f.verts[k]);
    }
    faces.push_back(face);
  }
}

/*
 * Read the vertex data for a number of frames.
 * The number of frames (F) is the first line of the file
 * This is followed by F blocks of V (num vertices) lines
 * each of which contains 3 comma separated floats representing X,Y,Z of that vertex.
 */
void
read_vert_data_for_frames(const std::string &anim_file_name,
                          unsigned int num_vertices,
                          std::vector<int> frame_indices,
                          std::vector<std::vector<Eigen::Vector3f>> &vertices_by_frame) {
  size_t curr_line_num = 0;
  std::vector<Eigen::Vector3f> frame_data;
  unsigned int num_frames_in_file = 0;
  unsigned int curr_frame_index_index = 0;
  unsigned int curr_frame_index = frame_indices[curr_frame_index_index];
  unsigned int current_frame_start_line = (curr_frame_index * num_vertices) + 1;
  process_file_by_lines(anim_file_name,
                        [&](const std::string &line) {
                          // First line
                          if (curr_line_num == 0) {
                            num_frames_in_file = std::stoi(line);
                            ++curr_line_num;
                            return;
                          }

                          // Bad frame number
                          if (curr_frame_index >= num_frames_in_file) {
                            throw std::runtime_error("Can't find frame " +
                                std::to_string(curr_frame_index) +
                                ". Only " +
                                std::to_string(num_frames_in_file) +
                                " frames in file.");
                          }

                          // Processed last frame I need to
                          if (curr_frame_index_index == frame_indices.size()) {
                            return;
                          }

                          // Not yet reached data for the curr frame
                          if (curr_line_num < current_frame_start_line) {
                            ++curr_line_num;
                            return;
                          }

                          // If we're still reading data for this frame, do so.
                          if (curr_line_num < current_frame_start_line + num_vertices) {
                            // Otherwise ... process this data
                            std::vector<std::string> coords = split(line, ',');
                            float x = stof(coords[0]);
                            float y = stof(coords[1]);
                            float z = stof(coords[2]);
                            frame_data.emplace_back(x, y, z);
                            ++curr_line_num;
                            // If we finished this frames data, stash it.
                            if (curr_line_num == current_frame_start_line + num_vertices) {
                              // We finished reading data
                              vertices_by_frame.push_back(frame_data);
                              frame_data.clear();
                              ++curr_frame_index_index;
                              if (curr_frame_index_index != frame_indices.size()) {
                                curr_frame_index = frame_indices[curr_frame_index_index];
                                current_frame_start_line = (curr_frame_index * num_vertices) + 1;
                              }
                            }
                            return;
                          }

                        }
  );
}

void
read_face_and_vertex_data(const std::string &ply_file_name,
                          std::vector<std::vector<std::size_t>> &faces,
                          std::vector<Eigen::Vector3f> &vertices) {
  FILE *f = fopen(ply_file_name.c_str(), "r");
  PlyFile *ply = read_ply(f);

  int elem_count;
  for (int i = 0; i < ply->num_elem_types; ++i) {
    // Prepare to read the i'th list of elements.
    auto elem_name = setup_element_read_ply(ply, i, &elem_count);
    if (strcmp("vertex", elem_name) == 0) {
      read_vertices(ply, i, elem_count, vertices);
    }
    if (strcmp("face", elem_name) == 0) {
      read_faces(ply, i, elem_count, faces);
    }
  }

  close_ply(ply);
  free_ply(ply);
}

/*
 * @return a vector[num_vertices] or adjacent vertex indices.
 */
std::vector<std::vector<std::size_t>>
compute_vertex_adjacency_from_faces(const std::vector<std::vector<std::size_t>> &faces,
                                    unsigned int num_vertices) {
  using namespace std;

  spdlog::info("Computing vertex adjacency");

  vector<vector<size_t>> adjacency;
  adjacency.resize(num_vertices);

  for (const auto &face: faces) {
    adjacency[face.back()].push_back(face.front());
    adjacency[face.front()].push_back(face.back());

    for (int i = 1; i < face.size() - 1; i++) {
      adjacency[face[i]].push_back(face[i - 1]);
      adjacency[face[i]].push_back(face[i + 1]);
    }
  }
  return adjacency;
}

/*
 * @return a vector[num_vertices] of the face indices of adjacent faces.
 */
std::vector<std::vector<std::size_t>>
compute_face_adjacency_from_faces(const std::vector<std::vector<std::size_t>> &faces,
                                  unsigned int num_vertices) {
  using namespace std;

  vector<vector<size_t>> adjacency;
  adjacency.resize(num_vertices);

  for (int face_index = 0; face_index < faces.size(); ++face_index) {
    const auto &face = faces[face_index];
    for (unsigned long i: face) {
      adjacency[i].push_back(face_index);
    }
  }
  return adjacency;
}

Eigen::Vector3f normal_from_vertices(
    const Eigen::Vector3f &v1,
    const Eigen::Vector3f &v2,
    const Eigen::Vector3f &v3
) {
  auto vec1 = (v2 - v1).normalized();
  auto vec2 = (v3 - v2).normalized();
  auto normal = vec1.cross(vec2).normalized();
  return normal;
}

/*
 * @return a vector[num_frames] of vectors[num_faces] or normals
 */
std::vector<std::vector<Eigen::Vector3f>>
compute_face_normals(const std::vector<std::vector<std::size_t>> &faces,
                     const std::vector<std::vector<Eigen::Vector3f>> &vertex_data_by_frame) {
  using namespace std;
  using namespace Eigen;
  using namespace spdlog;

  assert(!vertex_data_by_frame.empty());
  assert(!vertex_data_by_frame[0].empty());
  assert(!faces.empty());

  spdlog::info("Computing face normals for {} faces", faces.size());
  vector<vector<Vector3f>> face_normals;

  // Reserve space for normals in each frame
  auto num_frames = vertex_data_by_frame.size();
  face_normals.resize(num_frames);

  // For each face
  for (const auto &face: faces) {
    // Extract three vertices
    auto v1_index = face[0];
    auto v2_index = face[1];
    auto v3_index = face[2];

    for (int frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
      const auto &vec1 = vertex_data_by_frame[frame_idx][v1_index];
      const auto &vec2 = vertex_data_by_frame[frame_idx][v2_index];
      const auto &vec3 = vertex_data_by_frame[frame_idx][v3_index];
      Vector3f face_normal = normal_from_vertices(vec1, vec2, vec3);
      face_normals[frame_idx].push_back(face_normal);
    }
  }
  return face_normals;
}

/*
 * @return a vector[num_frames] of vectors[num_vertcies] of norma
 */
std::vector<std::vector<Eigen::Vector3f>>
compute_vertex_normals(
    const std::vector<std::vector<std::size_t>> &faces,
    const std::vector<std::vector<Eigen::Vector3f>> &vertex_data_by_frame
) {
  spdlog::info("Computing vertex normals");

  using namespace std;
  auto num_frames = vertex_data_by_frame.size();

  // vector[num_frames] of vectors[num_faces] of normals
  auto face_normals = compute_face_normals(faces, vertex_data_by_frame);

  // vector[num_vertices] of vector[] of adj vertex indices
  auto vertex_face_adjacency = compute_face_adjacency_from_faces(faces, vertex_data_by_frame[0].size());
  auto num_vertices = vertex_face_adjacency.size();

  // Normal per vertex per frame
  vector<vector<Eigen::Vector3f>> vertex_normals_by_frame;
  vertex_normals_by_frame.resize(num_frames);

  // For each frame
  for (auto frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
    // For each vertex
    for (int vertex_id = 0; vertex_id < num_vertices; ++vertex_id) {
      // Compute mean of face normals
      Eigen::Vector3f mean{0, 0, 0};
      for (const auto &face_id: vertex_face_adjacency[vertex_id]) {
        const auto &face_normal = face_normals[frame_idx][face_id];
        mean += face_normal;
      }
      vertex_normals_by_frame[frame_idx].push_back(mean.normalized());
    }
  }
  return vertex_normals_by_frame;
}

void centre_frame_data(std::vector<std::vector<Eigen::Vector3f>> &frame_data) {
  using namespace std;
  using namespace Eigen;

  auto num_frames = frame_data.size();
  auto num_vertices = frame_data[0].size();

  for (auto frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
    Vector3f centroid{0, 0, 0};
    for (auto vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx) {
      centroid += frame_data[frame_idx][vertex_idx];
    }
    centroid /= static_cast<float>(num_vertices);
    for (auto vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx) {
      frame_data[frame_idx][vertex_idx] -= centroid;
    }
  }
}

void scale_frame_data(std::vector<std::vector<Eigen::Vector3f>> &frame_data) {
  using namespace std;
  using namespace Eigen;

  auto num_frames = frame_data.size();
  auto num_vertices = frame_data[0].size();

  Vector3f minv{MAXFLOAT, MAXFLOAT, MAXFLOAT};
  Vector3f maxv{-MAXFLOAT, -MAXFLOAT, -MAXFLOAT};

  for (auto vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx) {
    auto point = frame_data[0][vertex_idx];
    if (point.x() < minv.x())
      minv.x() = point.x();
    if (point.y() < minv.y())
      minv.y() = point.y();
    if (point.z() < minv.z())
      minv.z() = point.z();
    if (point.x() > maxv.x())
      maxv.x() = point.x();
    if (point.y() > maxv.y())
      maxv.y() = point.y();
    if (point.z() > maxv.z())
      maxv.z() = point.z();
  }
  auto dist = (maxv - minv).norm();
  auto scale = 30.0f / dist;
  for (auto frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
    for (auto vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx) {
      frame_data[frame_idx][vertex_idx] *= scale;
    }
  }
}

void read_data(const args &args,
               unsigned int &num_vertices,
               std::vector<std::vector<std::size_t>> &faces,
               std::vector<std::vector<Eigen::Vector3f>> &vertex_data_by_frame
) {
  std::vector<Eigen::Vector3f> vertices;
  read_face_and_vertex_data(args.ply_file_name, faces, vertices);
  num_vertices = vertices.size();
  read_vert_data_for_frames(args.anim_file_name, num_vertices, args.frames, vertex_data_by_frame);
}

int main(int argc, char *argv[]) {
  using namespace std;

  auto args = parse_args(argc, argv);

  unsigned int num_frames = args.frames.size();
  spdlog::info("Reading from {} and {}. Writing {} frames to {}",
               args.ply_file_name, args.anim_file_name,
               (num_frames == 0)
               ? "all"
               : to_string(num_frames),
               args.surfel_file_name
  );

  unsigned int num_vertices;
  vector<vector<size_t>> faces;
  vector<vector<Eigen::Vector3f>> vertex_data_by_frame;
  read_data(args, num_vertices, faces, vertex_data_by_frame);
  spdlog::info("Read {} vertices and positions for {} frames.",
               num_vertices, num_frames
  );

  // Normalise and centre
  spdlog::info("Normalising data");
  centre_frame_data(vertex_data_by_frame);
  scale_frame_data(vertex_data_by_frame);

  // Compute vertex normals by frame
  auto vertex_normals = compute_vertex_normals(faces, vertex_data_by_frame);

  // create a surfel graph
  SurfelGraphPtr g = make_shared<SurfelGraph>();

  std::mt19937 rnd{123};
  auto surfel_builder = new SurfelBuilder(rnd);
  vector<SurfelGraphNodePtr> node_for_vertex;

  spdlog::info("Building graph");

  spdlog::info("Adding nodes");
  for (int vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx) {
    surfel_builder
        ->reset()
        ->with_id("s_" + to_string(vertex_idx));

    for (unsigned int frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
      surfel_builder->with_frame(
          {0, 0, frame_idx},
          0.0f,
          vertex_normals[frame_idx][vertex_idx],
          vertex_data_by_frame[frame_idx][vertex_idx]
      );
    }
    auto n = g->add_node(make_shared<Surfel>(surfel_builder->build()));
    node_for_vertex.push_back(n);
  }

  // Create neighbours
  auto vertex_vertex_adjacency = compute_vertex_adjacency_from_faces(faces, num_vertices);
  spdlog::info("Adding edges");
  for (int vertex_idx = 0; vertex_idx < num_vertices; ++vertex_idx) {
    for (auto adjacent_vertex_idx: vertex_vertex_adjacency[vertex_idx]) {
      try {
        g->add_edge(node_for_vertex[vertex_idx], node_for_vertex[adjacent_vertex_idx], SurfelGraphEdge{1.0f});
      } catch (runtime_error &e) {

      }
    }
  }

  // Save to file
  save_surfel_graph_to_file(args.surfel_file_name, g);
}