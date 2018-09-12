#pragma once

#include <Eigen/Core>
#include <map>

namespace animesh {

class PointNormal {
public:
	PointNormal( Eigen::Vector3f point, Eigen::Vector3f normal);

	using Ptr=std::shared_ptr<PointNormal>;

	/**
	 * @return The point.
	 */
	inline const Eigen::Vector3f& point() { return m_point; }

	/** 
	 * @return The normal.
	 */
	inline const Eigen::Vector3f& normal() { return m_normal; }

private:
	Eigen::Vector3f 	m_point;
	Eigen::Vector3f 	m_normal;
};

}