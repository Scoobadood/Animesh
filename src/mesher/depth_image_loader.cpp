#include <Eigen/Core>
#include <FileUtils/PgmFileParser.h>
#include <FileUtils/FileUtils.h>
#include <Camera/Camera.h>
#include <DepthMap/DepthMap.h>
#include "depth_image_loader.h"

static int DEBUG_LEVEL = 1;

/**
 * Read a depth image.
 */
void
read_depth_file(const std::string& 					file_name,
				unsigned int& 						width, 
				unsigned int& 						height,
				std::vector<std::vector<float>>&	depth_data) {
	using namespace std;

	width = 0;
	process_file_by_lines( file_name, 
		[&](const string& text_line){
			using namespace std;
			vector<float> depth_image_row;
			vector<string> tokens = tokenise(text_line);
			if( width == 0 ) {
				width = tokens.size();
			} else {
				if( width != tokens.size() ) {
					string message = "Lines of file must all be the same length";
					throw std::domain_error( message );
				}
			}
			for( auto token : tokens ) {
				float f = stof(token);
				depth_image_row.push_back(f);
			}
			depth_data.push_back(depth_image_row);
	}); 
	height = depth_data.size();
}


/**
 * Load one depth image point cloud.
 * @param file_name The file from which to load - expected to be a PGM file.
 * @param neighbour_indices The indices of neighbouring points as a vector for each point in the cloud.
 * @param points_with_normal Populated by this method.
 */
void
load_depth_image(const std::string& 						file_name,
				 std::vector<PointWithNormal2_5D>&   		points_with_normals,
				 std::vector<std::vector<unsigned int>>& 	neighbour_indices) {
	using namespace std;
	using namespace Eigen;

	points_with_normals.clear();
	neighbour_indices.clear();

	// Read the depth map
	DepthMap dm{file_name};

	// Remove unreliables
	dm.cull_unreliable_depths(8.0f,3.0f);

	// Get normals
	vector<vector<vector<float>>> normals = dm.get_normals( );

	// Compute index for each valid point
	vector<vector<int>> point_indices;
	point_indices.resize(dm.rows());
	int count = 0;
	for( int row = 0; row < dm.rows(); ++row ) {
		point_indices[row].resize(dm.cols());
		for( int col = 0; col < dm.cols(); ++col ) {
			if(dm.depth_at(row,col) != 0.0f ) {
				point_indices[row][col] = count++;
			} else {
				point_indices[row][col] = -1;
			}
		}
	}

	// Now build the data
	for( int row = 0; row < dm.rows(); ++row ) {
		for( int col = 0; col < dm.cols(); ++col ) {
			float nx = normals[row][col][0];
			float ny = normals[row][col][1];
			float nz = normals[row][col][2];
			if( nx+ny+nz > 0.0f ) {
				points_with_normals.push_back(PointWithNormal2_5D{
					Vector2f{col, row},
					dm.depth_at(row, col),
					Vector3f{nx, ny, nz}
				});
			}

			// Neighbours
			vector<unsigned int> these_neighbours;
			for( int ri = row - 1; ri <= row + 1; ++ri ) {
				for( int ci = col - 1; ci <= col + 1; ++ci ) {
					if( ri < 0 || ri >= dm.rows() || ci < 0 || ci >= dm.cols() )
						continue;
					int point_index = point_indices[ri][ci];
					if( point_index < 0 )
						continue;
					these_neighbours.push_back(point_index);
				}
			}
			neighbour_indices.push_back(these_neighbours);
		}
	}
}

/**
 * Read depth images and convert to 2.5D clouds
 * @param file_names The input file names of depth images, assumed to be PGM files.
 * @param point_clouds A vector which is populated by this method. For each frame, for each point, a PointWithNormal.
 * @param neighbours Populated by this method. For each frame, for each point, the neighbouring point indices used to compute the normal.
 */
void 
load_depth_images(const std::vector<std::string>& 						file_names,
				  std::vector<std::vector<PointWithNormal2_5D>>& 		point_clouds,
				  std::vector<std::vector<std::vector<unsigned int>>>&	neighbours) {
	using namespace std;

	// Clear out any residual data
	point_clouds.clear();
	neighbours.clear();

	int current_frame_index = 0;
	for( auto file_name : file_names ) {
		vector<PointWithNormal2_5D> points_with_normals;
		vector<vector<unsigned int>> neighbour_indices;

		// Load a single file
		load_depth_image(file_name, points_with_normals, neighbour_indices);

		++current_frame_index;

		point_clouds.push_back( points_with_normals );
		neighbours.push_back(neighbour_indices);
	}
}
