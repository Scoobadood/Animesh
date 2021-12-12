//
// Created by Dave Durbin on 29/11/21.
//

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <Eigen/Core>


template <typename Out>
void split(const std::string &s, char delim, Out result) {
    using namespace std;

    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) {
    using namespace std;

    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

Eigen::Vector3f string_to_vec3f( const std::string& s );
