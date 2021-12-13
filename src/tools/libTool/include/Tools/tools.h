#pragma once

#include <map>
#include <vector>
#include <string>
#include <Eigen/Eigen>

/**
 * Load pointcloud from file
 */
std::vector<Eigen::Vector3f>
load_pointcloud(const std::string& pointcloud_filename);

/**
 * Load all pointclouds from a directory given a regex pattern.
 * The regex should include one capture group that identifies the frame
 * number. This is assumed to be an integer and will be used as the key in the returned map.
 */
std::map<unsigned int, std::vector<Eigen::Vector3f>>
load_pointclouds(const std::string& directory, const std::string& regex);
