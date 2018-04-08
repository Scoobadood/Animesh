#pragma once

#include <Eigen/Core>
#include <vector>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/features/normal_3d.h>
#include <pcl/io/pcd_io.h>

struct Point {
	Eigen::Vector3f		location;
	Eigen::Vector3f		normal;
	Point( const Eigen::Vector3f& location, const Eigen::Vector3f& normal ) : location{ location}, normal{ normal } {}
};

/*
 * PointCloud is an abstract class representing an immutable collection of points
 */
class PointCloud {
public:
	static PointCloud * load_from_file( const std::string& file_name );

	PointCloud() : points {new pcl::PointCloud<pcl::PointXYZ>}, normals {new pcl::PointCloud<pcl::Normal>} {}

	PointCloud(pcl::PointCloud<pcl::PointXYZ>::Ptr points) : points{points}, normals {new pcl::PointCloud<pcl::Normal>} {};

	/**
	 * @return the size of the point cloud (number of points).
	 */
	size_t size( ) const;

	/**
	 * Add a point
	 */
	void add_point( Eigen::Vector3f point );

	/**
	 * Subscript operator
	 */
	const Point point( size_t index ) const;

	/**
	 * Compute the normals for this point cloud
	 */
	void compute_normals( );

private:
	static PointCloud * load_from_pcd_file( const std::string& file_name );
	static PointCloud * load_from_obj_file( const std::string& file_name );

	pcl::PointCloud<pcl::PointXYZ>::Ptr points;
	pcl::PointCloud<pcl::Normal>::Ptr normals;
};

/*
 * Case Insensitive Implementation of endsWith()
 * It checks if the string 'mainStr' ends with given string 'toMatch'
 */
bool ends_with_case_insensitive(const std::string& main_str, const std::string& to_match);