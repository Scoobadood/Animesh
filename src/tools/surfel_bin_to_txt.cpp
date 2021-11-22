
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <FileUtils/FileUtils.h>

#include <fstream>

int main(int argc, const char *argv[]) {
  std::string in_file = argv[1];
  unsigned short flags;
  std::default_random_engine rng{123};
  auto s = load_surfel_graph_from_file(in_file, flags, rng);

  std::string out_file;
  if (argc == 3) {
    out_file = argv[2];
  } else {
    auto name_and_extension = get_file_name_and_extension(in_file);
    out_file = name_and_extension.first + ".txt";
  }
  std::ofstream txt_file{out_file};
  Eigen::Vector3f vertex, tangent, normal;
  txt_file << "# id, pos_x, pos_y, pos_z, tan_x, tan_y, tan_z, norm_x, norm_y, norm_z, posy_u, posy_v" << std::endl;
  for (auto n: s->node_data()) {
    n->get_vertex_tangent_normal_for_frame(0, vertex, tangent, normal);
    txt_file << n->id();
    txt_file << ", " << vertex.x() << ", " << vertex.y() << ", " << vertex.z() << ", ";
    txt_file << tangent.x() << ", " << tangent.y() << ", " << tangent.z() << ", ";
    txt_file << normal.x() << ", " << normal.y() << ", " << normal.z() << ", ";
    txt_file << n->reference_lattice_offset().x() << ", " << n->reference_lattice_offset().y() << std::endl;
  }
}