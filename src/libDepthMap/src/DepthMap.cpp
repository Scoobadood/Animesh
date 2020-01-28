#include <DepthMap/DepthMap.h>
#include <FileUtils/FileUtils.h>

#include <vector>
#include <cmath>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <Eigen/Dense> // For cross product
#include "../../libCamera/include/Camera/Camera.h"
DepthMap::DepthMap(const std::string &filename) {
    using namespace std;

    m_width = 0;
    m_height = 0;

    vector<vector<float>> rows;
    process_file_by_lines(filename,
                          [&](const string &text_line) {
                              using namespace std;
                              vector<float> depth_image_row;
                              vector<string> tokens = tokenise(text_line);
                              if (m_width == 0) {
                                  m_width = tokens.size();
                              } else {
                                  if (m_width != tokens.size()) {
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
    m_height = rows.size();

    m_depth_data = new float[m_width * m_height];
    for (unsigned int y = 0; y < m_height; ++y) {
        for (unsigned int x = 0; x < m_width; ++x) {
            m_depth_data[index(x, y)] = rows.at(y).at(x);
        }
    }
}


/**
 * Construct from an array of floats and dimensions
 * @param width The number of columns provided.
 * @param height The number of rows provided.
 * @param depth_data a rows*cols, row major set of depths.
 */
DepthMap::DepthMap(unsigned int width, unsigned int height, float *depth_data) {
    m_width = width;
    m_height = height;
    unsigned int num_entries = width * height;
    m_depth_data = new float[num_entries];
    if( depth_data != nullptr ) {
        memcpy(m_depth_data, depth_data, num_entries * sizeof(float));
    } else {
        for( int i=0; i<num_entries; ++i ) {
            m_depth_data[i] = 0.0f;
        }
    }
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

/**
 * Given an (X,Y) coordinate in a depth map, return a set of flags indicating which
 * directions from this coordinate are valid (in the sense that they fall inside the
 * depth map.
 * @param x X coordinate.
 * @param y Y coordinate.
 * @param eightConnected If true, consider diagonally adjacent pixels, otherwise just horizontal and vertical.
 * @return
 */
unsigned int
DepthMap::get_valid_directions(unsigned int x, unsigned int y, bool eightConnected) const {
    unsigned int flags = 0;
    if (y > 0) {
        flags |= UP;
        if (eightConnected) {
            if (x > 0) flags |= UP_LEFT;
            if (x < width() - 1) flags |= UP_RIGHT;
        }
    }
    if (y < height() - 1) {
        flags |= DOWN;
        if (eightConnected) {
            if (x > 0) flags |= DOWN_LEFT;
            if (x < width() - 1) flags |= DOWN_RIGHT;
        }
    }
    if (x > 0) {
        flags |= LEFT;
    }
    if (x < width() - 1) {
        flags |= RIGHT;
    }
    return flags;
}

inline bool DepthMap::flag_is_set(int flags, DepthMap::tDirection flag) {
    return ((flags & flag) == flag);
}

inline int clear_flag_if_zero(float value, int flags, DepthMap::tDirection flag) {
    if (value != 0.0f) return flags;
    return flags & (~flag);
}

/**
 * Given a row and column, get the depths of adjacent pixels in the depth map.
 * If eightConnected is true, consider diagonal neighbours, otherwise only left irght up and down.
 * Populates neighbour_depths with depth values (or 0.0) and returns a flag in indicating
 * which values are valid.
 */
int
DepthMap::get_neighbour_depths(unsigned int x, unsigned int y, float neighbour_depths[],
                               bool eightConnected) const {
    int flags = get_valid_directions(x, y, eightConnected);
    float d;
    if (flag_is_set(flags, UP)) {
        d = depth_at(x, y - 1);
        flags = clear_flag_if_zero(d, flags, UP);
        neighbour_depths[0] = d;
    }
    if (flag_is_set(flags, DOWN)) {
        d = depth_at(x, y + 1);
        flags = clear_flag_if_zero(d, flags, DOWN);
        neighbour_depths[1] = d;
    }
    if (flag_is_set(flags, LEFT)) {
        d = depth_at( x - 1, y);
        flags = clear_flag_if_zero(d, flags, LEFT);
        neighbour_depths[2] = d;
    }
    if (flag_is_set(flags, RIGHT)) {
        d = depth_at(x + 1, y);
        flags = clear_flag_if_zero(d, flags, RIGHT);
        neighbour_depths[3] = d;
    }
    if (eightConnected) {
        if (flag_is_set(flags, UP_LEFT)) {
            d = depth_at(x - 1, y - 1);
            flags = clear_flag_if_zero(d, flags, UP_LEFT);
            neighbour_depths[4] = d;
        }
        if (flag_is_set(flags, UP_RIGHT)) {
            d = depth_at(x - 1, y + 1);
            flags = clear_flag_if_zero(d, flags, UP_RIGHT);
            neighbour_depths[5] = d;
        }
        if (flag_is_set(flags, DOWN_LEFT)) {
            d = depth_at(x + 1, y - 1);
            flags = clear_flag_if_zero(d, flags, DOWN_LEFT);
            neighbour_depths[6] = d;
        }
        if (flag_is_set(flags, DOWN_RIGHT)) {
            d = depth_at(x + 1, y + 1);
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
    bool reliable[m_height][m_width];
    for (int r = 0; r < m_height; ++r) {
        for (int c = 0; c < m_width; ++c) {
            reliable[r][c] = false;
        }
    }

    for (unsigned int y = 1; y < m_height - 1; ++y) {
        for (unsigned int x = 1; x < m_width - 1; ++x) {
            float p = m_depth_data[index(x, y)];

            // Handle existing non-depth values
            if (p == 0.0f) {
                reliable[y][x] = false;
                continue;
            }


            /* Compute
                 Hp = |D(r,c-1) - D(r,c+1)|
                 Vp = |D(r-1,c) - D(r+1,c)|
             */
            float dp[] = {
                    m_depth_data[index(x, y - 1)],
                    m_depth_data[index(x, y + 1)],
                    m_depth_data[index(x - 1, y)],
                    m_depth_data[index(x + 1, y)]
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
            reliable[y][x] = rel;
        }
    }

    // Remove unreliable pixels
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (!reliable[y][x]) {
                m_depth_data[index(x, y)] = 0.0f;
            }
        }
    }
}


/**
 * Compute normals that have all adjacent data set in the depth map,
 * that is, those normals which can be computed fully from depth data alone.
 * // TODO: An alternative approach is here: https://cromwell-intl.com/3d/normals.html
 * // Currently using this: https://stackoverflow.com/questions/34644101/calculate-surface-normals-from-depth-image-using-neighboring-pixels-cross-produc
 *
 *
 */
void
DepthMap::compute_natural_normals(const Camera& camera) {
    using namespace std;


    // For each row
    for (int y = 0; y < height(); ++y) {

        // Temp storage for the y's type and normals
        vector<vector<float>> current_row_normals;
        vector<tNormal> current_row_normal_types;

        // Compute the scale factor from depths to pixels


        // For each column
        for (int x = 0; x < width(); ++x) {
            float d = depth_at(x, y);

            // If depth is 0 then there's no normal to be had here.
            if (d == 0.0f) {
                current_row_normal_types.push_back(NONE);
                current_row_normals.push_back(vector<float>{0, 0, 0});
                continue;
            }

            float neighbour_depths[4];
            int neighbours_present = get_neighbour_depths(x, y, neighbour_depths, false);

            // If there are not four neighbours then I have a derived normal
            if (neighbours_present != FOUR) {
                current_row_normal_types.push_back(DERIVED);
                current_row_normals.push_back(vector<float>{0, 0, 0});
                continue;
            }

            // Backproject the depths
            // TODO: This is super slow and we sill be calculating the same points over and over
            // We should avoid this if possible by back projecting all points once

            // Otherwise I have a natural normal
            Eigen::Vector3f a = camera.to_world_coordinates(x + 1, y, neighbour_depths[3]);
            Eigen::Vector3f b = camera.to_world_coordinates(x - 1, y, neighbour_depths[2]);
            Eigen::Vector3f c1 = a - b;
//            float dzdx = (c[2] / c[0]);
//            float dzdx = (neighbour_depths[3] - neighbour_depths[2]) / 2.0f;
            a = camera.to_world_coordinates(x, y + 1, neighbour_depths[1]);
            b = camera.to_world_coordinates(x, y - 1, neighbour_depths[0]);
            Eigen::Vector3f c2 = a - b;
//            float dzdy = (c[2] / c[0]);
//            float dzdy = (neighbour_depths[1] - neighbour_depths[0]) / 2.0f;
//            float scale = sqrt(dzdx * dzdx + dzdy * dzdy + 1.0f);
//            dzdx /= scale;
//            dzdy /= scale;
            auto n = c1.cross(c2);
            n.normalize();
            vector<float> norm{n(0), n(1), n(2)};
            current_row_normals.push_back(norm);
            current_row_normal_types.push_back(NATURAL);
        }
        normals.push_back(current_row_normals);
        normal_types.push_back(current_row_normal_types);
    }
}

void
DepthMap::compute_derived_normals() {
    using namespace std;

    for (size_t row = 0; row < height(); ++row) {
        for (size_t col = 0; col < width(); ++col) {
            // Skip existing normals
            if (normal_types.at(row).at(col) == NONE
                || normal_types.at(row).at(col) == NATURAL) {
                continue;
            }

            vector<float> sum{0.0f, 0.0f, 0.0f};
            int count = 0;
            for (int ri = row - 1; ri <= row + 1; ri++) {
                for (int ci = col - 1; ci <= col + 1; ci++) {
                    if (ri < 0 || ri >= width() || ci < 0 || ci >= width()) {
                        continue;
                    }
                    if (normal_types.at(ri).at(ci) == NONE) {
                        continue;
                    }
                    if (normal_types.at(ri).at(ci) == NATURAL) {
                        float nx = normals.at(ri).at(ci)[0];
                        float ny = normals.at(ri).at(ci)[1];
                        float nz = normals.at(ri).at(ci)[2];
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
                normals.at(row).at(col)[0] = mean_nx / l;
                normals.at(row).at(col)[1] = mean_ny / l;
                normals.at(row).at(col)[2] = mean_nz / l;
            }
        }
    }
}


/**
 * Return the normals. Compute them if needed.
 */
const std::vector<std::vector<std::vector<float>>> &
DepthMap::get_normals() const {
    using namespace std;
    if (normals.size() == 0) {
        throw runtime_error("Normals not calculated. Call compute_normalsd() first");
    }
    return normals;
}

/**
 * Return true if the normal at the given coordinates is defined, i.e.
 * has a non-0 length.
 * @param x The x coordinate to check
 * @param y The y coordinate to chec
 */
bool
DepthMap::is_normal_defined(unsigned int x, unsigned int y) const {
    using namespace std;
    vector<float> n = get_normals().at(y).at(x);
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

    unsigned int new_height = height() / 2;
    unsigned int new_width = width() / 2;
    auto new_data = new float[new_height * new_width];

    struct PixelCoord {
        unsigned int x;
        unsigned int y;

        PixelCoord(unsigned int x, unsigned int y) : x{x}, y{y} {}
    };

    for (unsigned int y = 0; y < new_height; ++y) {
        for (unsigned int x = 0; x < new_width; ++x) {
            unsigned int source_y = y * 2;
            unsigned int source_x = x * 2;
            float values[4]{
                depth_at(source_x, source_y),
                depth_at(min(source_x + 1, width() - 1), source_y),
                depth_at(source_x, min(source_y + 1, height() - 1)),
                depth_at(min(source_x + 1, width() - 1), min(source_y + 1, height() - 1))
            };
            new_data[y * new_width + x] = merge(values);
        }
    }
    return DepthMap{new_width, new_height, new_data};
}

DepthMap::NormalWithType DepthMap::normal_at(unsigned int x, unsigned int y) const {
    NormalWithType n{normal_types.at(y).at(x), normals.at(y).at(x).at(0), normals.at(y).at(x).at(1), normals.at(y).at(x).at(2) };
    return n;
}
/**
 * Compute the surface normals for each point in the
 * depth map.
 */
void
DepthMap::compute_normals(const Camera& camera) {
    using namespace std;


    compute_natural_normals(camera);
    compute_derived_normals();


    // Validation
    int natural_norm_count = 0;
    int derived_norm_count = 0;
    int zero_norms = 0;
    for (int y = 0; y < height(); ++y) {
        for (int x = 0; x < width(); ++x) {
            auto norm_type = normal_types.at(y).at(x);
            if (norm_type == NONE) {
                zero_norms++;
                continue;
            }
            if (norm_type == DERIVED) {
                derived_norm_count++;
            } else {
                natural_norm_count++;
            }
            // Check that norm is legal
            const auto& normal = normals.at(y).at(x);
            float norm_length = sqrt(normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2]);
            if (isnan(norm_length)) {
                cout << "Nan " << ((norm_type == DERIVED) ? "derived" : "natural" ) << " normal at y:" << y << ", x:" << x << endl;
            } else
            if (norm_length == 0.0) {
                cout << "Zero " << ((norm_type == DERIVED) ? "derived" : "natural") << " normal at y:" << y << ", x:" << x << endl;
            } else
            if (abs(norm_length - 1.0) > 1e-3) {
                cout << "Non unit " << ((norm_type == DERIVED) ? "derived" : "natural") << " normal (" << norm_length << ") at y:" << y << ", x:" << x << endl;
            }
            if( norm_length == 0.0) zero_norms++;
        }
    }
    if (derived_norm_count + natural_norm_count < 5) {
        cout << "Suspiciously low normal counts derived: " << derived_norm_count << ", natural:" << natural_norm_count
             << endl;
    }
    int num_norms = normal_types.size() * normal_types.at(0).size();
    if( ((zero_norms * 100) / num_norms) > 95) {
        cout << "Suspiciously high zero norms : " << zero_norms << " out of  " << num_norms << endl;
    }
}

