
#include <Surfel/SurfelGraph.h>
#include <Surfel/Surfel_IO.h>
#include <FileUtils/FileUtils.h>

#include <fstream>

int main(int argc, const char *argv[]) {
  std::string in_file = argv[1];
  std::mt19937 rng;         // the Mersenne Twister with a popular choice of parameters
  uint32_t seed_val = 123;  // populate somehow
  rng.seed(seed_val);

  auto s = load_surfel_graph_from_file(in_file, rng);

  std::string out_file;
  if (argc == 3) {
    out_file = argv[2];
  } else {
    auto name_and_extension = get_file_name_and_extension(in_file);
    out_file = name_and_extension.first + "x3.bin";
  }
  for (auto &n: s->node_data()) {
    for (auto &f: n->frame_data()) {
      f.position *= 3.0f;
    }
  }
  save_surfel_graph_to_file(out_file, s);
}