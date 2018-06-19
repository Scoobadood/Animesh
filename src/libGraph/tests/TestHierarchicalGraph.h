
std::string merge_strings( const std::string& s1, const std::string& s2 );

class TestHierarchicalGraph : public ::testing::Test {
public:



    animesh::Graph<std::string, std::string>::GraphNode * gn1;
    animesh::Graph<std::string, std::string>::GraphNode * gn2;
	animesh::Graph<std::string, std::string> graph{ merge_strings };


	void SetUp( );
	void TearDown( );
};