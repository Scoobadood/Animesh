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

char *elem_names[] = { /* list of the elements in the object */
    "vertex", "face"
};

PlyProperty vert_props[] = { /* list of property information for a vertex */
    {"x", Float32, Float32, 0, 0, 0, 0, 0},
    {"y", Float32, Float32, 4, 0, 0, 0, 0},
    {"z", Float32, Float32, 8, 0, 0, 0, 0},
};

PlyProperty face_props[] = { /* list of property information for a face */
    {"vertex_indices", Int32, Int32, offsetof(Face, verts),
     1, Uint8, Uint8, offsetof(Face, nverts)},
};

void extract_vertices(PlyFile *ply, std::vector<Eigen::Vector3f> &vertices) {
  int elem_count;
  for (int i = 0; i < ply->num_elem_types; ++i) {
    // Prepare to read the i'th list of elements.
    auto elem_name = setup_element_read_ply(ply, i, &elem_count);
    if (strcmp("vertex", elem_name) != 0) {
      continue;
    }

    // Set up for getting vertex elements.
    setup_property_ply(ply, &vert_props[0]);
    setup_property_ply(ply, &vert_props[1]);
    setup_property_ply(ply, &vert_props[2]);

    for (int j = 0; j < elem_count; j++) {
      Eigen::Vector3f v;
      get_element_ply(ply, (float *) v.data());
      vertices.push_back(v);
    }
  }
}
void extract_faces(PlyFile *ply, std::vector<std::vector<std::size_t>> &faces) {
  int elem_count;
  for (int i = 0; i < ply->num_elem_types; ++i) {
    // Prepare to read the i'th list of elements.
    auto elem_name = setup_element_read_ply(ply, i, &elem_count);
    if (strcmp("face", elem_name) != 0) {
      continue;
    }

    // Set up for getting face elements
    setup_property_ply(ply, &face_props[0]);
    for (int j = 0; j < elem_count; ++j) {
      Face f;
      get_element_ply(ply, (void *) &f);
      std::vector<std::size_t> face;
      for (int k = 0; k < f.nverts; ++k) {
        face.push_back(f.verts[k]);
      }
      faces.push_back(face);
    }
  }
}

void
read_vert_data_for_frames(const std::string &anim_file_name, unsigned int num_vertices,
                          std::vector<int> frame_indices,
                          std::vector<std::vector<Eigen::Vector3f>> &vertices_by_frame) {
  // Read all future positions of vertices from txt file for given frames
  // This file contains a single line containing thenumber of frames then
  // N x num_verts lines
  size_t curr_line_num = 0;
  std::vector<Eigen::Vector3f> frame_data;
  unsigned int num_frames_in_file = 0;
  unsigned int next_frame_index = 0;
  unsigned int next_frame = frame_indices[next_frame_index];
  unsigned int next_frame_line_num = ((next_frame - 1) * num_vertices) + 1;
  process_file_by_lines(anim_file_name,
                        [&](const std::string &line) {
                          // First line
                          if (curr_line_num == 0) {
                            num_frames_in_file = std::stoi(line);
                            ++curr_line_num;
                            return;
                          }

                          // Bad frame number
                          if (next_frame >= num_frames_in_file) {
                            throw std::runtime_error("Can't find frame " +
                                std::to_string(next_frame) +
                                ". Only " +
                                std::to_string(num_frames_in_file) +
                                " frames in file.");
                          }

                          // Processed last frame I need to
                          if (next_frame_index == frame_indices.size()) {
                            return;
                          }

                          // Not yet reached data for the curr frame
                          if (curr_line_num < next_frame_line_num) {
                            ++curr_line_num;
                            return;
                          }

                          // Process this data
                          if (curr_line_num < next_frame_line_num + num_vertices) {
                            std::vector<std::string> coords = split(line, ',');
                            float x = stof(coords[0]);
                            float y = stof(coords[1]);
                            float z = stof(coords[2]);
                            frame_data.emplace_back(x, y, z);
                            ++curr_line_num;
                            return;
                          }

                          // Reached end of data
                          vertices_by_frame.push_back(frame_data);
                          frame_data.clear();
                          ++next_frame_index;
                          if (next_frame_index == frame_indices.size()) {
                            return; // Done
                          }

                          next_frame = frame_indices[next_frame_index];
                          next_frame_line_num = ((next_frame - 1) * num_vertices) + 1;
                          return;
                        });
}

void
read_verts_and_faces(const std::string &ply_file_name,
                     std::vector<Eigen::Vector3f> &vertices,
                     std::vector<std::vector<std::size_t>> &faces) {
  FILE *f = fopen(ply_file_name.c_str(), "r");
  PlyFile *ply = read_ply(f);

  extract_vertices(ply, vertices);
  extract_faces(ply, faces);
  close_ply(ply);
  free_ply(ply);
}

std::vector<std::vector<std::size_t>>
compute_vertex_adjacency_from_faces(const std::vector<std::vector<std::size_t>> &faces,
                                    unsigned int num_vertices) {
  std::vector<std::vector<std::size_t>> adjacency;
  adjacency.resize(num_vertices);
  for (const auto &face: faces) {
    for (int i = 0; i < face.size() - 1; i++) {
      adjacency[face[i]].push_back(face[i + 1]);
    }
    adjacency[face.back()].push_back(face[0]);
  }
  return adjacency;
}

std::vector<std::vector<std::size_t>>
compute_face_adjacency_from_faces(const std::vector<std::vector<std::size_t>> &faces,
                                  unsigned int num_vertices) {
  std::vector<std::vector<std::size_t>> adjacency;
  adjacency.resize(num_vertices);
  for (int face_index = 0; face_index < faces.size(); ++face_index) {
    const auto &face = faces[face_index];
    for (int i = 0; i < face.size(); i++) {
      adjacency[face[i]].push_back(face_index);
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

std::vector<std::vector<Eigen::Vector3f>>
compute_face_normals(const std::vector<std::vector<std::size_t>> &faces,
                     const std::vector<std::vector<Eigen::Vector3f>> &frames) {
  std::vector<std::vector<Eigen::Vector3f>> face_normals;
  face_normals.resize(frames.size());

  for (const auto &face: faces) {
    auto v1_index = face[0];
    auto v2_index = face[1];
    auto v3_index = face[2];

    for (int frame_idx = 0; frame_idx < frames.size(); ++frame_idx) {
      const auto &vec1 = frames[frame_idx][v1_index];
      const auto &vec2 = frames[frame_idx][v2_index];
      const auto &vec3 = frames[frame_idx][v3_index];

      face_normals[frame_idx].push_back(normal_from_vertices(vec1, vec2, vec3));
    }
  }
  return face_normals;
}

std::vector<std::vector<Eigen::Vector3f>>
compute_vertex_normals(
    const std::vector<std::vector<std::size_t>> &vertex_face_adjacency, // vertex -> vertex list
    const std::vector<std::vector<Eigen::Vector3f>> &face_normals) // frame -> normal_per_face
{

  // Normal per vertex per frame
  std::vector<std::vector<Eigen::Vector3f>> vertex_normals_by_frame;

  size_t num_frames = face_normals.size();
  size_t num_vertices = vertex_face_adjacency.size();
  vertex_normals_by_frame.resize(num_frames);

  // For each frame
  for( int frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
    // For each vertex
    for ( int vertex_id = 0; vertex_id < num_vertices; ++vertex_id ) {
      // Compute mean of face normals
      Eigen::Vector3f mean{0,0,0};
      for( const auto & face_id : vertex_face_adjacency[vertex_id] ) {
        const auto & face_normal = face_normals[frame_idx][face_id];
        mean += face_normal;
      }
      vertex_normals_by_frame[frame_idx].push_back(mean.normalized());
    }
  }
  return vertex_normals_by_frame;
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

  vector<Eigen::Vector3f> vertices;
  vector<vector<size_t>> faces;
  read_verts_and_faces(args.ply_file_name, vertices, faces);

  unsigned int num_vertices_per_frame = vertices.size();
  vector<vector<Eigen::Vector3f>> frames;
  frames.push_back(vertices);
  read_vert_data_for_frames(args.anim_file_name,
                            num_vertices_per_frame,
                            args.frames,
                            frames);


  // Convert faces to adjacency array of arrays
  auto vertex_vertex_adjacency = compute_vertex_adjacency_from_faces(faces, num_vertices_per_frame);

  // Compute face_vertex adjacency
  auto vertex_face_adjacency = compute_face_adjacency_from_faces(faces, num_vertices_per_frame);

  // Compute face normals (by frame)
  auto face_normals = compute_face_normals(faces, frames);

  // Compute vertex norms by frame
  auto vertex_normals = compute_vertex_normals(vertex_face_adjacency, face_normals);

  // create a surfel graph
  SurfelGraphPtr g = make_shared<SurfelGraph>();

  default_random_engine rnd{123};
  auto surfel_builder = new SurfelBuilder(rnd);
  vector<SurfelGraphNodePtr> node_for_vertex;

  for( int vertex_idx = 0; vertex_idx< num_vertices_per_frame; ++vertex_idx) {
    surfel_builder
        ->reset()
        ->with_id("s_" + to_string(vertex_idx));

    for( int frame_idx = 0; frame_idx < num_frames; ++frame_idx) {
      FrameData fd;
      // PixelInFrame
      fd.pixel_in_frame.pixel.x = 0;
      fd.pixel_in_frame.pixel.y = 0;
      fd.pixel_in_frame.frame = frame_idx;
      fd.depth = 0.0f;

      // Transform - Identity for now.
      fd.transform << 1, 0, 0, 0, 1, 0, 0, 0, 1;

      // Normal
      fd.normal = vertex_normals[frame_idx][vertex_idx];

      // Position
      fd.position = frames[frame_idx][vertex_idx];

      surfel_builder->with_frame(fd);
    }
    auto n = g->add_node(make_shared<Surfel>(surfel_builder->build()));
    node_for_vertex.push_back(n);
  }

  // Create neighbours
  for( int vertex_idx = 0; vertex_idx < num_vertices_per_frame; ++vertex_idx) {
    for( auto adjacent_vetrex_idx : vertex_vertex_adjacency[vertex_idx]) {
      try {
        g->add_edge(node_for_vertex[vertex_idx], node_for_vertex[adjacent_vetrex_idx], SurfelGraphEdge{1.0f});
      } catch( runtime_error& e) {

      }
    }
  }

  // Save to file
  save_surfel_graph_to_file(args.surfel_file_name, g);
}