//
// Created by Dave Durbin on 2019-08-11.
//

#include "../include/Properties/Properties.h"

#include <fstream>
#include <sstream>
#include <iostream>

std::string &ltrim(std::string &str, const std::string &chars = "\t\n\v\f\r ") {
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

std::string &rtrim(std::string &str, const std::string &chars = "\t\n\v\f\r ") {
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

std::string &trim(std::string &str, const std::string &chars = "\t\n\v\f\r ") {
    return ltrim(rtrim(str, chars), chars);
}

/**
 * File has format :
 * COMMENT | BLANK LINE | PROPERTY
 * COMMENT = '#'
 * PROPERTY = KEY SEPARATOR VALUE
 * SEPARATOR = '='
 * @param file_name
 */
Properties::Properties(const std::string &file_name) {
    using namespace std;

    ifstream in_file(file_name);
    for( string line; getline( in_file, line ); ) {
        // Trim the string to remove leading and trailing spaces
        line = trim(line);

        // Skip comments
        if( line.at(0) == '#' ) {
            continue;
        }

        // Find the '='
        size_t pos = line.find('=');
        if( pos >= line.length()) {
            throw runtime_error( "Invalid line in properties file");
        }
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        property_map.insert(make_pair(trim(key), trim(value)));
    }
}

const std::string &Properties::getProperty(const std::string &key) const {
    using namespace std;

    return property_map.at(key);
}

int Properties::getIntProperty(const std::string &key) const {
    return std::stoi(getProperty(key));
}

float Properties::getFloatProperty(const std::string &key) const {
    std::string val = getProperty(key);
    float f = std::stof(val);
    return f;
}
