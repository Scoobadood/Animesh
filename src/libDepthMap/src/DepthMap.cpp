#include <DepthMap/DepthMap.h>
#include <FileUtils/FileUtils.h>

#include <vector>
#include <cmath>

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

			bool r = true;
			if( hp > ts && vp > ts) {
				// First case: hp > ts && vp > ts ==> p is near ts
				// p is reliable if |Dpi - Dp| <= Tl for all i
				for( int i=0; i<4; i++ ) {
					if( fabsf(dp[i] - p) > tl) {
						r = false;
						break;
					}
				}
			}

			else if (vp > ts && hp <= tl) { 
				// Second case: p near horizontal discontinutiy. Check vertical neighbours for reliability
				for( int i=2; i<4; i++ ) {
					if( fabsf(dp[i] - p) > tl) {
						r = false;
						break;
					}
				}
			}

			else if ( hp > ts && vp <= tl) {
				// Third case: p near vertical discontinutiy. Check horizontal neighbours for reliability
				for( int i=0; i<2; i++ ) {
					if( fabsf(dp[i] - p) > tl) {
						r = false;
						break;
					}
				}
			}

			else {
				// Fourth case: Generally allpoints in homogeneous region but p may be an outlier check for this.
				for( int i=0; i<4; i++ ) {
					if( fabsf(dp[i] - p) > tl) {
						r = false;
						break;
					}
				}
			}
			reliable[r][c] = r;
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
