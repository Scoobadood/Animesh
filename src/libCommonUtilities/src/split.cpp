//
// Created by Dave Durbin on 29/11/21.
//

#include "../include/CommonUtilities/split.h"
#include <vector>
#include <string>

Eigen::Vector3f string_to_vec3f( const std::string& s ) {
    using namespace std;
    using namespace Eigen;

    vector<string> numbers = split(s, ',');
    assert( numbers.size() == 3);
    return Vector3f {
        stof(numbers[0]),
        stof(numbers[1]),
        stof(numbers[2])
    };
}