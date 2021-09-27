//
// Created by Dave Durbin (Old) on 26/9/21.
//

#include "TestPlyFileParser.h"
#include <GeomFileUtils/PlyFileParser.h>
#include <vector>

#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) EXPECT_THROW( \
        try { \
            stmt; \
        } catch (const etype& ex) { \
            EXPECT_EQ(std::string(ex.what()), whatstring); \
            throw; \
        } \
    , etype)

void TestPlyFileParser::TearDown() {}
void TestPlyFileParser::SetUp() {}

TEST_F(TestPlyFileParser, parse_fails_for_bad_magic) {
  using namespace std;

  string ply_string = R"(
xyz
comment A broken ply file
)";
  std::istringstream ply_data{ply_string};

  animesh::PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      std::runtime_error,
      "Expected \"ply\""
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_bad_format) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format asskey 1.0
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Invalid format \"asskey\""
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_missing_format) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
fomat ascii 1.0
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Expected \"format ... \""
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_bad_version) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format ascii 1.1
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Invalid version \"1.1\", expected 1.0"
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_missing_element) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format ascii 1.0
property float x
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Unexpected line \"property float x\"."
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_unsupported_element) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format ascii 1.0
element of_surprise 10
property float x
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Unknown element type \"of_surprise\"."
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_bad_property) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format ascii 1.0
element vertex 10
property x
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Property line is invalid \"property x\""
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_bad_property_type) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format ascii 1.0
element vertex 10
property wibble x
end_header
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Property type \"wibble\" is not valid"
  );
}

TEST_F(TestPlyFileParser, parse_fails_for_bad_list_property_type) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format ascii 1.0
element vertex 10
property list wibble float x
end_header
)";
  stringstream ply_data{ply_string};

  PlyFileParser pfp;
  EXPECT_THROW_WITH_MESSAGE(
      pfp.parse_stream(ply_data),
      runtime_error,
      "Property list type \"wibble\" is not valid"
  );
}

TEST_F(TestPlyFileParser, parse_handles_ascii_format) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format ascii 1.0
element vertex 10
property float x
property float y
end_header
)";
  stringstream ply_data{ply_string};

  auto header = parse_header(ply_data);
  EXPECT_EQ( 10, header.num_vertices);
  EXPECT_TRUE( header.is_ascii);
}

TEST_F(TestPlyFileParser, parse_handles_binary_big_endian_format) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format binary_little_endian 1.0
element vertex 10
property float x
property float y
end_header
)";
  stringstream ply_data{ply_string};

  auto header = parse_header(ply_data);
  EXPECT_EQ( 10, header.num_vertices);
  EXPECT_FALSE( header.is_bigendian);
  EXPECT_FALSE( header.is_ascii);
}

TEST_F(TestPlyFileParser, parse_handles_binary_little_endian_format) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format binary_big_endian 1.0
element vertex 10
property float x
property float y
end_header
)";
  stringstream ply_data{ply_string};

  auto header = parse_header(ply_data);
  EXPECT_EQ( 10, header.num_vertices);
  EXPECT_TRUE( header.is_bigendian);
  EXPECT_FALSE( header.is_ascii);
}

TEST_F(TestPlyFileParser, parse_handles_mulitple_elements) {
  using namespace std;
  using namespace animesh;

  string ply_string = R"(
ply
format binary_big_endian 1.0
element vertex 10
property float x
property float y
element face 1
property list uchar int vertex_index
end_header
)";
  stringstream ply_data{ply_string};

  auto header = parse_header(ply_data);
  EXPECT_EQ( 10, header.num_vertices);
  EXPECT_EQ( 1, header.num_faces);
  EXPECT_TRUE( header.is_bigendian);
  EXPECT_FALSE( header.is_ascii);
}