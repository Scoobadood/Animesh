#include "gtest/gtest.h"
#include "TestFileUtils.h"

void TestFileUtils::SetUp( ){
}

void TestFileUtils::TearDown( ) {}

TEST_F(TestFileUtils, FileNameExtensionWithFileAndExtension) {
  using namespace std;
    pair<string,string> actual = get_file_name_and_extension( "file.ext");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "ext", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithRelPathFileAndExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "path/file.ext");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "ext", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithAbsPathFileAndExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "/path/file.ext");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "ext", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithDottedPathFileAndExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "/pa.th/file.ext");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "ext", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithDottedPathFileNoExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "/pa.th/file");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithDottedPathFileEmptyExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "/pa.th/file.");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithFileNoExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "file");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithNoFileExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( ".ext");
  EXPECT_EQ( "", actual.first);
  EXPECT_EQ( "ext", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithRelPathFileNoExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "path/file");
  EXPECT_EQ( "file", actual.first);
  EXPECT_EQ( "", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithRelPathNoFileExtension) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( "path/.ext");
  EXPECT_EQ( "", actual.first);
  EXPECT_EQ( "ext", actual.second);
}

TEST_F(TestFileUtils, FileNameExtensionWithNeither) {
  using namespace std;
  pair<string,string> actual = get_file_name_and_extension( ".");
  EXPECT_EQ( "", actual.first);
  EXPECT_EQ( "", actual.second);
}
