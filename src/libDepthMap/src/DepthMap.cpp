#include <DepthMap/DepthMap.h>
#include <FileUtils/FileUtils.h>

#include <vector>
#include <cmath>
#include <iostream>

DepthMap::DepthMap(const std::string& filename) {
	using namespace std;

	width = 0;
	height = 0;

	vector<vector<float>> rows;
	process_file_by_lines( filename, 
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
			rows.push_back(depth_image_row);
	}); 
	height = rows.size();

	depth_data = new float[width * height];
	for( int r = 0; r < height; r++ ) {
		for( int c = 0; c < width; c++ ) {
			depth_data[index(r, c)] = rows.at(r).at(c);
		}
	}
}

float median_value(const std::vector<float>& v) {
	using namespace std;
	if( v.size() == 0 ) return 0;

	vector<float> tmp = v;
	sort( tmp.begin(), tmp.end());
	int sz = tmp.size();
	int mid = sz/2;
	if( sz % 2 == 0 ) {
		return (tmp[mid] + tmp[mid - 1]) / 2.0f;
	}
	return tmp[mid];
}

int 
DepthMap::get_valid_neighbours(unsigned int row, unsigned int col, bool eightConnected) const {
	int flags = 0;
	if( row > 0 ) {
		flags |= UP;
		if( eightConnected ) {
			if( col > 0 ) flags |= UP_LEFT;
			if( col < cols() - 1) flags |= UP_RIGHT;
		}
	}
	if( row < rows() - 1 ) {
		flags |= DOWN;
		if( eightConnected) {
			if( col > 0 ) flags |= DOWN_LEFT;
			if( col < cols() - 1) flags |= DOWN_RIGHT;
		}
	}
	if( col > 0 ) {
		flags |= LEFT;
	}
	if( col < cols() - 1 ) {
		flags |= RIGHT;
	}
	return flags;
}

inline bool DepthMap::flag_is_set( int flags, DepthMap::tDirection flag ) {
	return ((flags & flag) == flag);
}

inline int clear_flag_if_zero( float value, int flags, DepthMap::tDirection flag) {
	if( value != 0.0f) return flags;
	return flags & (!flag);
}

int 
DepthMap::get_neighbour_depths(unsigned int row, unsigned int col, float neighbour_depths[], bool eightConnected) const {
	int flags = get_valid_neighbours(row, col, eightConnected);
	float d;
	if( flag_is_set(flags, UP) ) {
		d = depth_at(row-1, col);
		flags = clear_flag_if_zero( d, flags, UP );
		neighbour_depths[0] = d;
	}
	if( flag_is_set(flags, DOWN)) {
		d = depth_at(row+1, col);
		flags = clear_flag_if_zero( d, flags, DOWN );
		neighbour_depths[1] = d;
	}
	if( flag_is_set(flags, LEFT)) {
		d = depth_at(row, col-1);
		flags = clear_flag_if_zero( d, flags, LEFT );
		neighbour_depths[2] = d;
	}
	if( flag_is_set(flags, RIGHT)) {
		d = depth_at(row, col+1);
		flags = clear_flag_if_zero( d, flags, RIGHT );
		neighbour_depths[3] = d;
	}
	if( eightConnected) {
		if( flag_is_set(flags, UP_LEFT)) {
			d = depth_at(row-1, col-1);
			flags = clear_flag_if_zero( d, flags, UP_LEFT );
			neighbour_depths[4] = d;
		}
		if( flag_is_set(flags, UP_RIGHT)) {
			d = depth_at(row-1, col+1);
			flags = clear_flag_if_zero( d, flags, UP_RIGHT );
			neighbour_depths[5] = d;
		}
		if( flag_is_set(flags, DOWN_LEFT)) {
			d = depth_at(row+1, col-1);
			flags = clear_flag_if_zero( d, flags, DOWN_LEFT );
			neighbour_depths[6] = d;
		}
		if( flag_is_set(flags, DOWN_RIGHT)) {
			d = depth_at(row+1, col+1);
			flags = clear_flag_if_zero( d, flags, DOWN_RIGHT );
			neighbour_depths[7] = d;
		}
	}
	return flags;
}

/*
 * Based on 
 * A Nonlocal Filter-Based Hybrid Strategy for Depth Map Enhancement
 */
void 
DepthMap::cull_unreliable_depths(float ts, float tl) {
	using namespace std;
	bool reliable[height][width];
	for( int r = 0; r < height; ++r ) {
		for( int c = 0; c < width; ++c ) {
			reliable[r][c] = false;
		}
	}

	for( int r = 1; r < height-1; ++r ) {
		for( int c = 1; c < width-1; ++c ) {
			float p = depth_data[index(r,c)];

			// Handle existing non-depth values
			if( p == 0.0f ) {
				reliable[r][c] = false;
				continue;
			}


			/* Compute
 				Hp = |D(r,c-1) - D(r,c+1)|
 				Vp = |D(r-1,c) - D(r+1,c)|
 			*/
 			float dp[] = {
 				depth_data[index(r,c-1)],
 				depth_data[index(r,c+1)],
 				depth_data[index(r-1,c)],
 				depth_data[index(r+1,c)]
 			};

			float hp = fabsf( dp[1] - dp[0]);
			float vp = fabsf( dp[3] - dp[2]);

			bool rel = false;
			if( hp > ts && vp > ts) {
				// First case: hp > ts && vp > ts ==> p is near ts
				// p is reliable if |Dpi - Dp| <= Tl for *any* i
				for( int i=0; i<4; i++ ) {
					if( fabsf(dp[i] - p) <= tl) {
						rel = true;
						break;
					}
				}
			}

			else if (vp > ts && hp <= tl) { 
				// Second case: p near VERTICAL discontinutiy. Check HORIZONTAL neighbours for reliability
				for( int i=0; i<2; i++ ) {
					if( fabsf(dp[i] - p) <= tl) {
						rel = true;
						break;
					}
				}
			}

			else if ( hp > ts && vp <= tl) {
				// Third case: p near HORIZONTAL discontinutiy. Check VERTICAL neighbours for reliability
				for( int i=2; i<4; i++ ) {
					if( fabsf(dp[i] - p) <= tl) {
						rel = true;
						break;
					}
				}
			}

			else {
				// Fourth case: Generally allpoints in homogeneous region but p may be an outlier check for this.
				for( int i=0; i<4; i++ ) {
					if( fabsf(dp[i] - p) <= tl) {
						rel = true;
						break;
					}
				}
			}
			reliable[r][c] = rel;
		}
	}

	// Remove unreliable pixels
	for( int r = 0; r < height; ++r ) {
		for( int c = 0; c < width; ++c ) {
			if( !reliable[r][c] ) {
				depth_data[index(r,c)] = 0.0f;
			}
		}
	}
}


/**
 * Compute normals that have all adjacent data set in the depth map,
 * that is, those normals which can be computed fully from depth data alone.
 */
void
DepthMap::compute_natural_normals( ) {
	using namespace std;

	for( int row = 0; row < rows(); ++row ) {
		vector<vector<float>> normal_row;
		vector<tNormal> normal_row_types;
		for( int col = 0; col < cols(); ++col) {
			float d = depth_at( row, col );
			float neighbour_depths[4];
			// If depth is invalid or we don;t have four neighbours, skip
			if( ( d == 0.0f) || get_neighbour_depths(row,col, neighbour_depths, false) != FOUR ) {
				normal_row.push_back(vector<float>{0,0,0});
			    if( d == 0.0f ) {
			    	normal_row_types.push_back(NONE);
			    } else {
			    	normal_row_types.push_back(DERIVED);
			    }
				continue;
			}

			float dzdx = neighbour_depths[3] - neighbour_depths[2];
			float dzdy = neighbour_depths[1] - neighbour_depths[0];
			float scale = sqrt(dzdx*dzdx + dzdy*dzdy + 1.0f);
			dzdx /= scale;
			dzdy /= scale;
			vector<float> norm{-dzdx, -dzdy, 1.0f / scale};
			normal_row.push_back(norm);
	    	normal_row_types.push_back(NATURAL);
		}
		normals.push_back(normal_row);
		normal_types.push_back(normal_row_types);
	}
}

void
DepthMap::compute_derived_normals( ) {
	using namespace std;

	for( int row = 0; row < rows(); ++row ) {
		vector<vector<float>> normal_row;
		for( int col = 0; col < cols(); ++col) {
			// Skip existing normals
			if(    normal_types[row][col] == NONE 
				|| normal_types[row][col] == NATURAL ) {
				continue;
			}

			vector<float> sum{0.0f, 0.0f, 0.0f};
			int count = 0;
			for( int ri = row - 1; ri <= row + 1; ri++ ) {
				for( int ci=col - 1; ci <= col + 1; ci++ ) {
					if( ri < 0 || ri >= cols() || ci < 0 || ci >= cols() ) {
						continue;
					}
					if( normal_types[ri][ci] == NONE) {
						continue;
					}
					if( normal_types[ri][ci] == NATURAL) {
						float nx = normals[ri][ci][0];
						float ny = normals[ri][ci][1];
						float nz = normals[ri][ci][2];
						sum[0] += nx;
						sum[1] += ny;
						sum[2] += nz;
						count++;
					}
				}
			}
			normals[row][col][0] = sum[0] / count;
			normals[row][col][1] = sum[1] / count;
			normals[row][col][2] = sum[2] / count;
		}
	}
}

/**
 * Compute the surface normals for each point in the
 * depth map.
 */
void
DepthMap::compute_normals() {
	compute_natural_normals();
	compute_derived_normals();
}

/**
 * Return the normals. Compute them if needed.
 */
const std::vector<std::vector<std::vector<float>>>& 
DepthMap::get_normals() {
	using namespace std;
	if( normals.size() == 0 ) { 
		compute_normals();
	}
	return normals;
}

