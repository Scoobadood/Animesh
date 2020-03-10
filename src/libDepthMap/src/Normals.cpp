//
// Created by Dave Durbin on 9/3/20.
//

#include <DepthMap/DepthMap.h>
#include <DepthMap/Normals.h>

void
validate_normals(const DepthMap* depth_map) {
    using namespace std;

    // Validation
    int natural_norm_count = 0;
    int derived_norm_count = 0;
    int zero_norms = 0;
    for (int y = 0; y < depth_map->height(); ++y) {
        for (int x = 0; x < depth_map->width(); ++x) {
            auto norm_type = depth_map->normal_at(x, y).type;
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
            const auto &normal = depth_map->normal_at(x, y);
            float norm_length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            if (isnan(norm_length)) {
                cout << "Nan " << ((norm_type == DERIVED) ? "derived" : "natural") << " normal at y:" << y << ", x:"
                     << x << endl;
            } else if (norm_length == 0.0) {
                cout << "Zero " << ((norm_type == DERIVED) ? "derived" : "natural") << " normal at y:" << y << ", x:"
                     << x << endl;
            } else if (abs(norm_length - 1.0) > 1e-3) {
                cout << "Non unit " << ((norm_type == DERIVED) ? "derived" : "natural") << " normal (" << norm_length
                     << ") at y:" << y << ", x:" << x << endl;
            }
            if (norm_length == 0.0) zero_norms++;
        }
    }
    if (derived_norm_count + natural_norm_count < 5) {
        cout << "Suspiciously low normal counts derived: " << derived_norm_count << ", natural:" << natural_norm_count
             << endl;
    }

    size_t num_norms = depth_map->get_normals().size() * depth_map->get_normals().at(0).size();
    if (((zero_norms * 100) / num_norms) > 95) {
        cout << "Suspiciously high zero norms : " << zero_norms << " out of  " << num_norms << endl;
    }
}