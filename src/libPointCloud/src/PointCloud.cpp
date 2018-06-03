#include <PointCloud/PointCloud.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/obj_io.h>
#include <pcl/PCLPointCloud2.h>
#include <FileUtils/FileUtils.h>
#include <string>
#include <tuple>
#include <Eigen/Core>


/**
 * Parse the string into tokens separated by delimeter
 * @param line The line
 * @param delimeter The delimeter
 * @return A vector of string tokens
 */
std::vector<std::string> parse_string_into_tokens( const std::string& line, const std::string& delimiter );

/**
 * Parse f line
 * @param line The line. Should begin with f<sp> 
 * @return 
 */
void parse_vertices_and_normals_from_f_line( const std::string& line, std::vector<int>& vertex_indices, std::vector<int>& normal_indices );

/**
 * Parse element from an f line. It should be of the form n or n/n or n//n or n/n/n
 * @param line The token.
 * @param index of vertices found in this token
 * @param index of normal found in this token
 */
int parse_vertex_and_normal_from_token( const std::string& line, int& vertex_index, int& normal_index );

/**
 * Parse a v - line into a vector of vertices
 */
void parse_v_line( std::string& line, std::vector<Eigen::Vector3f>& points );

/**
 * Parse a vn - line into a normal and store it
 */
void parse_vn_line( std::string& line, std::vector<Eigen::Vector3f>& normals );

/**
 * Parse a f - line into a normal and store it
 */
void parse_f_line( std::string& line, 
    std::vector<Eigen::Vector3f>& raw_points, 
    std::vector<Eigen::Vector3f>& raw_normals, 
    pcl::PointCloud<pcl::PointXYZ>::Ptr points, 
    pcl::PointCloud<pcl::Normal>::Ptr normals );

/**
 * Load a PointCloud from any supported file
 * @file_name The name of the file.
 * @return The PointCloud
 */
PointCloud * PointCloud::load_from_file( const std::string& file_name ) {
    if( ends_with_case_insensitive( file_name, "pcd") ) {
        return load_from_pcd_file( file_name );
    } else if ( ends_with_case_insensitive( file_name, "obj" ) ) {
        return load_from_obj_file( file_name );
    }
    return nullptr;
}



// PointCloud * PointCloud::load_from_obj_file2( const std::string& file_name ) {
//     using namespace std;
//     using namespace pcl;

//     if( file_name.size() == 0 ) 
//         throw std::invalid_argument( "Missing file name" );

//     bool is_directory;
//     if (!file_exists(file_name, is_directory ) )
//         throw std::runtime_error( "File not found: " + file_name );

//     vector<Eigen::Vector3f> raw_points;
//     vector<Eigen::Vector3f> raw_normals;
//     pcl::PointCloud<PointXYZ>::Ptr  points(new pcl::PointCloud<PointXYZ>);
//     pcl::PointCloud<Normal>::Ptr    normals(new pcl::PointCloud<Normal>);
//     process_file_by_lines( file_name, [&points, &normals, &raw_points, &raw_normals](std::string line) {
//         if( line[0] == 'v' ) {
//             if( line[1] == ' ' ) {
//                 parse_v_line( line, raw_points );
//             } else if( line[1] == 'n' ) {
//                 parse_vn_line( line, raw_normals );
//             }
//         } else if ( line[0] == 'f' ) {
//             parse_f_line( line, raw_points, raw_normals, points, normals);
//         }
//     });

//     std::cout << "Read data. Processing" << std::endl;

//     // If no points found, that's a problem
//     if( points->size() == 0 ) 
//         throw std::runtime_error( "No vertices in: " + file_name );

//     // Otherwise construct a pointcloud and either
//     // provide normals if we found any or else compute them
//     // if we didn't
//     PointCloud * pc= new PointCloud(points);
//     std::cout << "Made point cloud" << std::endl;
//     if( normals->size() == 0 ) {
//         pc->compute_normals();
//     } else {
//         pc->normals = normals;
//     }
//     std::cout << "Computed Normals" << std::endl;
//     return pc;
// }

PointCloud * PointCloud::load_from_obj_file( const std::string& file_name ) {
    using namespace std;
    using namespace pcl;

    if( file_name.size() == 0 ) 
        throw std::invalid_argument( "Missing file name" );

    bool is_directory;
    if (!file_exists(file_name, is_directory ) )
        throw std::runtime_error( "File not found: " + file_name );


    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
    if( pcl::io::loadOBJFile<pcl::PointXYZ> (file_name, *cloud) == -1) {
        PCL_ERROR ("Couldn't read OBJ file.pcd \n");
        return nullptr;
    }

    std::cout << "Loaded "
            << cloud->width * cloud->height
            << " data points from test_pcd.pcd with the following fields: "
            << std::endl;

    PointCloud * pc = new PointCloud( cloud );
    pc->compute_normals();

    return pc;
}


/**
 * Load a PointCloud from a PCD file
 * @file_name The name of the file.
 * @return The a pointer to the PointCloud or nullptr
 */
PointCloud * PointCloud::load_from_pcd_file( const std::string& file_name ) {
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);

    if ( pcl::io::loadPCDFile<pcl::PointXYZ> (file_name, *cloud) == -1) {
        PCL_ERROR ("Couldn't read PCD file.pcd \n");
        return nullptr;
    }

    std::cout << "Loaded "
            << cloud->width * cloud->height
            << " data points from test_pcd.pcd with the following fields: "
            << std::endl;
    for (size_t i = 0; i < cloud->points.size (); ++i)
    std::cout << "    " << cloud->points[i].x
              << " "    << cloud->points[i].y
              << " "    << cloud->points[i].z << std::endl;

    PointCloud * pc = new PointCloud( cloud );
    pc->compute_normals();

    return pc;
}

/**
 * @return the size of the point cloud (number of points).
 */
size_t PointCloud::size( ) const {
    return points->size();
}

/**
 * Add a point
 */
void PointCloud::add_point( Eigen::Vector3f point ) {
    pcl::PointXYZ p;
    p.x = point[0]; p.y = point[1]; p.z = point[2];
    points->push_back( p );
}

/**
 * Subscript operator
 */
const Point PointCloud::point( size_t index ) const {
    return Point{
        Eigen::Vector3f{ points->points[index].x * 100, points->points[index].y * 100, points->points[index].z * 100},
        Eigen::Vector3f{ normals->points[index].normal_x, normals->points[index].normal_y, normals->points[index].normal_z},
    };
}

/**
 * Compute the normals for this point cloud
 */
void PointCloud::compute_normals( ) {
    using namespace pcl;

    // Build normals
    // Create the normal estimation class, and pass the input dataset to it
    NormalEstimation<PointXYZ, Normal> ne;
    ne.setInputCloud (points);

    // Create an empty kdtree representation, and pass it to the normal estimation object.
    // Its content will be filled inside the object, based on the given input dataset (as no other search surface is given).
    search::KdTree<PointXYZ>::Ptr tree (new search::KdTree<PointXYZ> ());
    ne.setSearchMethod (tree);
    ne.setViewPoint( 0.0f, 0.0f, 10.0f );

    // Use 10 neghbours
    ne.setKSearch (10);

    // Compute the features
    ne.compute (*normals);
}


/*
 * Case Insensitive Implementation of endsWith()
 * It checks if the string 'mainStr' ends with given string 'toMatch'
 */
bool ends_with_case_insensitive(const std::string& main_str, const std::string& to_match)
{
    auto it = to_match.begin();
    return main_str.size() >= to_match.size() &&
            std::all_of(std::next(main_str.begin(),main_str.size() - to_match.size()), main_str.end(), [&it](const char & c){
                return ::tolower(c) == ::tolower(*(it++))  ;
    } );
}

/**
 * Parse the string into tokens separated by delimeter
 * @param line The line
 * @param delimeter The delimeter
 * @return A vector of string tokens
 */
std::vector<std::string> parse_string_into_tokens( const std::string& line, const std::string& delimiter ) {
    using namespace std;

    vector<string> tokens;
    string s = line;
    size_t pos = 0;
    string token;
    while ((pos = s.find(delimiter)) != string::npos) {
        token = s.substr(0, pos);
        tokens.push_back( token );
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back( s );
    return tokens;
}

/**
 * Parse f line
 * @param line The line. Should begin with f<sp> 
 * @return 
 */
void parse_vertices_and_normals_from_f_line( const std::string& line, std::vector<int>& vertex_indices, std::vector<int>& normal_indices ) {
    using namespace std;

    vector<string> tokens = parse_string_into_tokens(line, " ");

    // Should be an f and at least 3 vertex/texture/normal elements but maybe 4
    if( tokens.size() < 4 ) 
        throw invalid_argument( "Not enough elements in f line: " + line );

    // Only consider 1 to 3
    for( int i=1; i <= 3; ++i) {
        int vertex_index;
        int normal_index;
        int found = parse_vertex_and_normal_from_token( tokens[i], vertex_index, normal_index );
        if( found & 0x01) vertex_indices.push_back( vertex_index );
        if( found & 0x02) normal_indices.push_back( normal_index );
    }
}

/**
 * Parse element from an f line. It should be of the form n or n/n or n//n or n/n/n
 * @param line The token.
 * @param index of vertices found in this token
 * @param index of normal found in this token
 */
int parse_vertex_and_normal_from_token( const std::string& line, int& vertex_index, int& normal_index ) {
    using namespace std;

    int found_flags = 0;

    vector<string> values = parse_string_into_tokens(line, "/");
    if( values.size() > 0 ) {
        vertex_index = stoi( values[0]);
        found_flags = 1;
    }
    if( values.size() > 2 ) {
        normal_index = stoi( values[2]);
        found_flags |= 2;
    }

    return found_flags;
}

/**
 * Parse a v - line into a vector of vertices
 */
void parse_v_line( std::string& line, std::vector<Eigen::Vector3f>& points ) {
    using namespace std;

    istringstream sstream{line};
    char v;
    float x, y, z;
    sstream >> v >> x >> y >> z;
    points.push_back( Eigen::Vector3f{x, y, z} );
}

/**
 * Parse a vn - line into a normal and store it
 */
void parse_vn_line( std::string& line, std::vector<Eigen::Vector3f>& normals ) {
    using namespace std;

    istringstream sstream{line};
    char v;
    float nx, ny, nz;
    sstream >> v >> v >> nx >> ny >> nz;
    normals.push_back( Eigen::Vector3f{nx, ny, nz}.normalized() );
}

/**
 * Parse a f - line into a normal and store it
 */
void parse_f_line( 
    std::string& line, 
    std::vector<Eigen::Vector3f>& raw_points, 
    std::vector<Eigen::Vector3f>& raw_normals, 
    pcl::PointCloud<pcl::PointXYZ>::Ptr points, 
    pcl::PointCloud<pcl::Normal>::Ptr normals ) {

    using namespace std;

    vector<int> vertex_indices;
    vector<int> normal_indices;
    parse_vertices_and_normals_from_f_line( line, vertex_indices, normal_indices );
    if( vertex_indices.size() < 3 )
        throw invalid_argument("Not enough vertex indices in: " + line);
    if( ( normal_indices.size() < 3 ) && ( normal_indices.size() != 0 ) )
        throw invalid_argument("Not enough normal indices in: " + line);
    for( int i=0; i < vertex_indices.size(); ++i ) {
        int vi = vertex_indices[i] - 1;
        if( vi < 0 || vi > raw_points.size() )
            throw invalid_argument("Vertex index out of range in: " + line);
        Eigen::Vector3f v = raw_points[vi];
        points->push_back( pcl::PointXYZ{ v.x(), v.y(), v.z() });
        if( normal_indices.size() > 0) {
            int ni = normal_indices[i] - 1;
            if( ni < 0 || ni > raw_normals.size() )
                throw invalid_argument("Normal index out of range in: " + line);
            Eigen::Vector3f n = raw_normals[ni];
            normals->push_back( pcl::Normal{n.x(), n.y(), n.z() });
        }
    }
}