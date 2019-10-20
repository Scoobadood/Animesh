#include <DepthMap/DepthMap.h>
#include <FileUtils/FileUtils.h>

#include <vector>
#include <cmath>
#include <iostream>
#include <cstring>
#include <algorithm>

DepthMap::DepthMap(const std::string &filename) {
    using namespace std;

    width = 0;
    height = 0;

    vector<vector<float>> rows;
    process_file_by_lines(filename,
                          [&](const string &text_line) {
                              using namespace std;
                              vector<float> depth_image_row;
                              vector<string> tokens = tokenise(text_line);
                              if (width == 0) {
                                  width = tokens.size();
                              } else {
                                  if (width != tokens.size()) {
                                      string message = "Lines of file must all be the same length";
                                      throw std::domain_error(message);
                                  }
                              }
                              for (auto token : tokens) {
                                  float f = stof(token);
                                  depth_image_row.push_back(f);
                              }
                              rows.push_back(depth_image_row);
                          });
    height = rows.size();

    depth_data = new float[width * height];
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            depth_data[index(r, c)] = rows.at(r).at(c);
        }
    }
}


/**
 * Construct from an array of floats and dimensions
 * @param rows The number of rows provided.
 * @param cols The number of columns provided.
 * @param depth_data a rows*cols, row major set of depths.
 */
DepthMap::DepthMap(unsigned int rows, unsigned int cols, float *depth_data) {
    this->width = cols;
    this->height = rows;
    unsigned int num_entries = rows * cols;
    this->depth_data = new float[num_entries];
    memcpy(this->depth_data, depth_data, num_entries * sizeof(float));
}


float median_value(const std::vector<float> &v) {
    using namespace std;
    if (v.size() == 0) return 0;

    vector<float> tmp = v;
    sort(tmp.begin(), tmp.end());
    int sz = tmp.size();
    int mid = sz / 2;
    if (sz % 2 == 0) {
        return (tmp[mid] + tmp[mid - 1]) / 2.0f;
    }
    return tmp[mid];
}

int
DepthMap::get_valid_directions(unsigned int row, unsigned int col, bool eightConnected) const {
    int flags = 0;
    if (row > 0) {
        flags |= UP;
        if (eightConnected) {
            if (col > 0) flags |= UP_LEFT;
            if (col < cols() - 1) flags |= UP_RIGHT;
        }
    }
    if (row < rows() - 1) {
        flags |= DOWN;
        if (eightConnected) {
            if (col > 0) flags |= DOWN_LEFT;
            if (col < cols() - 1) flags |= DOWN_RIGHT;
        }
    }
    if (col > 0) {
        flags |= LEFT;
    }
    if (col < cols() - 1) {
        flags |= RIGHT;
    }
    return flags;
}

inline bool DepthMap::flag_is_set(int flags, DepthMap::tDirection flag) {
    return ((flags & flag) == flag);
}

inline int clear_flag_if_zero(float value, int flags, DepthMap::tDirection flag) {
    if (value != 0.0f) return flags;
    return flags & (!flag);
}

/**
 * Given a row and column, get the depths of adjacent pixels in the depth map.
 * If eightConnected is true, consider diagonal neighbours, otherwise only left irght up and down.
 * Populates neighbour_depths with depth values (or 0.0) and returns a flag in indicating
 * which values are valid.
 */
int
DepthMap::get_neighbour_depths(unsigned int row, unsigned int col, float neighbour_depths[],
                               bool eightConnected) const {
    int flags = get_valid_directions(row, col, eightConnected);
    float d;
    if (flag_is_set(flags, UP)) {
        d = depth_at(row - 1, col);
        flags = clear_flag_if_zero(d, flags, UP);
        neighbour_depths[0] = d;
    }
    if (flag_is_set(flags, DOWN)) {
        d = depth_at(row + 1, col);
        flags = clear_flag_if_zero(d, flags, DOWN);
        neighbour_depths[1] = d;
    }
    if (flag_is_set(flags, LEFT)) {
        d = depth_at(row, col - 1);
        flags = clear_flag_if_zero(d, flags, LEFT);
        neighbour_depths[2] = d;
    }
    if (flag_is_set(flags, RIGHT)) {
        d = depth_at(row, col + 1);
        flags = clear_flag_if_zero(d, flags, RIGHT);
        neighbour_depths[3] = d;
    }
    if (eightConnected) {
        if (flag_is_set(flags, UP_LEFT)) {
            d = depth_at(row - 1, col - 1);
            flags = clear_flag_if_zero(d, flags, UP_LEFT);
            neighbour_depths[4] = d;
        }
        if (flag_is_set(flags, UP_RIGHT)) {
            d = depth_at(row - 1, col + 1);
            flags = clear_flag_if_zero(d, flags, UP_RIGHT);
            neighbour_depths[5] = d;
        }
        if (flag_is_set(flags, DOWN_LEFT)) {
            d = depth_at(row + 1, col - 1);
            flags = clear_flag_if_zero(d, flags, DOWN_LEFT);
            neighbour_depths[6] = d;
        }
        if (flag_is_set(flags, DOWN_RIGHT)) {
            d = depth_at(row + 1, col + 1);
            flags = clear_flag_if_zero(d, flags, DOWN_RIGHT);
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
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            reliable[r][c] = false;
        }
    }

    for (int r = 1; r < height - 1; ++r) {
        for (int c = 1; c < width - 1; ++c) {
            float p = depth_data[index(r, c)];

            // Handle existing non-depth values
            if (p == 0.0f) {
                reliable[r][c] = false;
                continue;
            }


            /* Compute
                 Hp = |D(r,c-1) - D(r,c+1)|
                 Vp = |D(r-1,c) - D(r+1,c)|
             */
            float dp[] = {
                    depth_data[index(r, c - 1)],
                    depth_data[index(r, c + 1)],
                    depth_data[index(r - 1, c)],
                    depth_data[index(r + 1, c)]
            };

            float hp = fabsf(dp[1] - dp[0]);
            float vp = fabsf(dp[3] - dp[2]);

            bool rel = false;

            // First case: hp > ts && vp > ts ==> p is near discontinuity
            // p is reliable if |Dpi - Dp| <= Tl for *any* i
            if (hp > ts && vp > ts) {
                for (int i = 0; i < 4; i++) {
                    if (fabsf(dp[i] - p) <= tl) {
                        rel = true;
                        break;
                    }
                }
            }

                // Second case: p near HORIZONTAL discontinutiy. Check VERTICAL neighbours for reliability
                // i.e. we want this pixel to be part of either the upper or lower region, not straddling.
            else if (vp > ts && hp <= tl) {
                for (int i = 2; i < 4; i++) {
                    if (fabsf(dp[i] - p) <= tl) {
                        rel = true;
                        break;
                    }
                }
            }

                // Third case: p near VERTICAL discontinutiy. Check HORIZONTAL neighbours for reliability
                // i.e. we want this pixel to be part of either the left or right region, not straddling.
            else if (hp > ts && vp <= tl) {
                for (int i = 0; i < 2; i++) {
                    if (fabsf(dp[i] - p) <= tl) {
                        rel = true;
                        break;
                    }
                }
            }

                // Fourth case: Generally all points in homogeneous region but p may be an outlier check for this.
            else {
                for (int i = 0; i < 4; i++) {
                    if (fabsf(dp[i] - p) <= tl) {
                        rel = true;
                        break;
                    }
                }
            }
            reliable[r][c] = rel;
        }
    }

    // Remove unreliable pixels
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            if (!reliable[r][c]) {
                depth_data[index(r, c)] = 0.0f;
            }
        }
    }
}


/**
 * Compute normals that have all adjacent data set in the depth map,
 * that is, those normals which can be computed fully from depth data alone.
 * // TODO: An alternative approach is here: https://cromwell-intl.com/3d/normals.html
 *
 */
void
DepthMap::compute_natural_normals() {
    using namespace std;

    for (int row = 0; row < rows(); ++row) {
        vector<vector<float>> normal_row;
        vector<tNormal> normal_row_types;
        for (int col = 0; col < cols(); ++col) {
            float d = depth_at(row, col);

            // if depth is 0 then there's no normal to be had here.
            if (d == 0.0f) {
                normal_row_types.push_back(NONE);
                normal_row.push_back(vector<float>{0, 0, 0});
                continue;
            }


            float neighbour_depths[4];
            int neighbours_present = get_neighbour_depths(row, col, neighbour_depths, false);

            // If there are not four neighbours then I have a derived normal
            if (neighbours_present != FOUR) {
                normal_row_types.push_back(DERIVED);
                normal_row.push_back(vector<float>{0, 0, 0});
                continue;
            }

            // Otherwise I have a natural normal
            float dzdx = neighbour_depths[3] - neighbour_depths[2];
            float dzdy = neighbour_depths[1] - neighbour_depths[0];
            float scale = sqrt(dzdx * dzdx + dzdy * dzdy + 1.0f);
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
DepthMap::compute_derived_normals() {
    using namespace std;

    for (size_t row = 0; row < rows(); ++row) {
        for (size_t col = 0; col < cols(); ++col) {
            // Skip existing normals
            if (normal_types[row][col] == NONE
                || normal_types[row][col] == NATURAL) {
                continue;
            }

            vector<float> sum{0.0f, 0.0f, 0.0f};
            int count = 0;
            for (int ri = row - 1; ri <= row + 1; ri++) {
                for (int ci = col - 1; ci <= col + 1; ci++) {
                    if (ri < 0 || ri >= cols() || ci < 0 || ci >= cols()) {
                        continue;
                    }
                    if (normal_types[ri][ci] == NONE) {
                        continue;
                    }
                    if (normal_types[ri][ci] == NATURAL) {
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
            // If count == 0; kill this one
            if( count == 0 ) {
                normal_types.at(row).at(col) = NONE;
            } else {
                float mean_nx = sum[0] / count;
                float mean_ny = sum[1] / count;
                float mean_nz = sum[2] / count;
                float l = sqrt(mean_nx * mean_nx + mean_ny * mean_ny + mean_nz * mean_nz);
                normals[row][col][0] = mean_nx / l;
                normals[row][col][1] = mean_ny / l;
                normals[row][col][2] = mean_nz / l;
            }
        }
    }
}

/**
 * Compute the surface normals for each point in the
 * depth map.
 */
void
DepthMap::compute_normals() {
    using namespace std;

    compute_natural_normals();
    compute_derived_normals();


    // Validation
    int natural_norm_count = 0;
    int derived_norm_count = 0;
    for (int r = 0; r < rows(); ++r) {
        for (int c = 0; c < cols(); ++c) {
            auto norm_type = normal_types.at(r).at(c);
            if (norm_type == NONE) {
                continue;
            }
            if (norm_type == DERIVED) {
                derived_norm_count++;
            } else {
                natural_norm_count++;
            }
            // Check that norm is legal
            auto normal = normals.at(r).at(c);
            float norm_length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
            if (isnan(norm_length)) {
                cout << "Nan " << ((norm_type == DERIVED) ? "derived" : "natural" ) << " normal at r:" << r << ", c:" << c << endl;
            } else
            if (norm_length == 0.0) {
                cout << "Zero " << ((norm_type == DERIVED) ? "derived" : "natural") << " normal at r:" << r << ", c:" << c << endl;
            } else
            if (abs(norm_length - 1.0) > 1e-3) {
                cout << "Non unit " << ((norm_type == DERIVED) ? "derived" : "natural") << " normal ("<<norm_length<<") at r:" << r << ", c:" << c << endl;
            }
        }
    }
    if (derived_norm_count + natural_norm_count < 5) {
        cout << "Suspiciously low normal counts derived: " << derived_norm_count << ", natural:" << natural_norm_count
             << endl;
    }
    //
}

/**
 * Return the normals. Compute them if needed.
 */
const std::vector<std::vector<std::vector<float>>> &
DepthMap::get_normals() const {
    using namespace std;
    if (normals.size() == 0) {
        (const_cast<DepthMap *>(this))->compute_normals();
    }
    return normals;
}

/**
 * Return true if the normal at the given coordinates is defined, i.e.
 * has a non-0 length.
 * @param row The row in the depth map for the normal
 * @param col The column in the depth map for the normal
 */
bool
DepthMap::is_normal_defined(unsigned int row, unsigned int col) const {
    using namespace std;
    if (normals.size() == 0) {
        (const_cast<DepthMap *>(this))->compute_normals();
    }
    vector<float> n = normals.at(row).at(col);
    return ((n.at(0) + n.at(0) + n.at(2)) != 0.0f);
}

/**
 * Merge four values into a single depth value to propagate up.
 * Compute mean of non-zero values.
 */
static float
merge(const float *source_values) {
    float sum = 0.0f;
    int count = 0;
    for (int i = 0; i < 4; ++i) {
        if (source_values[i] > 0.0f) {
            sum += source_values[i];
            count++;
        }
    }
    return count != 0 ? (sum / count) : 0;
}

/**
 * Subsample a depth map and return a map that is half the size (rounded down) in each dimension.
 * Entries in the resulting map are computed from the mean of entries in this map.
 */
DepthMap
DepthMap::resample() const {
    using namespace std;

    unsigned int new_rows = rows() / 2;
    unsigned int new_cols = cols() / 2;
    auto new_data = new float[new_rows * new_cols];

    struct PixelCoord {
        unsigned int row;
        unsigned int col;

        PixelCoord(unsigned int r, unsigned int c) : row(r), col(c) {}
    };

    for (unsigned int r = 0; r < new_rows; ++r) {
        for (unsigned int c = 0; c < new_cols; ++c) {
            unsigned int source_row = r * 2;
            unsigned int source_col = c * 2;
            float values[4];
            vector<PixelCoord> mapped_pixels;

            mapped_pixels.emplace_back(source_row, source_col);
            mapped_pixels.emplace_back(min(source_row + 1, rows() - 1), source_col);
            mapped_pixels.emplace_back(source_row, min(source_col + 1, cols() - 1));
            mapped_pixels.emplace_back(min(source_row + 1, rows() - 1), min(source_col + 1, cols() - 1));

            for (int i = 0; i < 4; ++i) {
                values[i] = depth_at(mapped_pixels[i].row, mapped_pixels[i].col);
            }
            // TODO: Merge knows we skip 0s and so should be responsible for ignoring mappings.
            new_data[r * new_cols + c] = merge(values);
        }
    }
    return DepthMap{new_rows, new_cols, new_data};
}

