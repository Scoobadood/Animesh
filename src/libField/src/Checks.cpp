#include <Field/Checks.h>
#include <Eigen/Core>

#include <string>

const float EPSILON = 1e-4;

/**
 * Check that a vector is of unit length or throw
 */
void checkUnitLength( const std::string& vector_name, const Eigen::Vector3f& vector ) {
	using namespace std;
	
	assert ( abs(vector.norm() - 1.0) <= EPSILON );
}

/**
 * Check that two vectors are perpendicular or throw
 */
void checkPerpendicular( const std::string& vec1_name, const Eigen::Vector3f& vec1, const std::string& vec2_name, const Eigen::Vector3f& vec2 ) {
	using namespace std;

	assert ( abs(vec1.dot( vec2) ) <= EPSILON );
}

void checkRotationMatrix(const std::string& matrix_name, const Eigen::Matrix3f& R) {
    using namespace std;
    using namespace Eigen;

    Matrix3f expectI = R * R.transpose();
    assert(abs(expectI(0,0) - 1.0f) < EPSILON);
    assert(abs(expectI(1,1) - 1.0f) < EPSILON);
    assert(abs(expectI(2,2) - 1.0f) < EPSILON);
}