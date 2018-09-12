#include "Field/PointNormal.h"
#include "Field/Checks.h"
#include <Eigen/Core>

using animesh::PointNormal;

PointNormal::PointNormal( Eigen::Vector3f point, Eigen::Vector3f normal) {
	checkNotZeroVector("Normal", normal);
	m_point = point;

	m_normal = normal.normalized();
}