add_library(
		RoSy SHARED
		include/RoSy/RoSy.h src/RoSy.cpp
		include/RoSy/RoSyOptimiser.h src/RoSyOptimiser.cpp
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
		RoSy PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE include/RoSy
)

target_link_libraries(
		RoSy
		Correspondence
		Geom
		Surfel
		Utilities
)

# Tests
add_executable(
		testRoSy
		tests/main.cpp
		tests/TestAngleBetweenVectors.cpp
		tests/TestMinimiseKL.cpp tests/TestMinimiseKL.h
		tests/TestVectorRotation.cpp tests/TestVectorRotation.h
		tests/GtestUtility.h)

target_link_libraries(
		testRoSy
		Geom
		RoSy
		gtest
		gmock)

add_test(
		NAME ZeroLengthVectorsShouldThrow
		COMMAND testRoSy --gtest_filter=ZeroLengthVectorsShouldThrow
)
add_test(
		NAME IdenticalVectorsShouldReturnZero
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.IdenticalVectorsShouldReturnZero
)
add_test(
		NAME TestVectorAngle45DegreesShouldReturnPIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test45DegreesShouldReturnPIOverFour
)
add_test(
		NAME TestVectorAngle90DegreesShouldReturnPIOverTwo
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test90DegreesShouldReturnPIOverTwo
)
add_test(
		NAME TestVectorAngle135DegreesShouldReturnThreePIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test135DegreesShouldReturnThreePIOverFour
)
add_test(
		NAME TestVectorAngle180DegreesShouldReturnPI
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test180DegreesShouldReturnPI
)
add_test(
		NAME TestVectorAngle225DegreesShouldReturnThreePIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test225DegreesShouldReturnThreePIOverFour
)
add_test(
		NAME TestVectorAngle270DegreesShouldReturnPIOverTwo
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test270DegreesShouldReturnPIOverTwo
)
add_test(
		NAME TestVectorAngle315DegreesShouldReturnPIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test315DegreesShouldReturnPIOverFour
)
add_test(
		NAME TestVectorAngle360DegreesShouldReturnZero
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test360DegreesShouldReturnZero
)
add_test(
		NAME TestVectorAngle45DegreesInYPlaneShouldReturnPIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test45DegreesInYPlaneShouldReturnPIOverFour
)
add_test(
		NAME TestVectorAngle90DegreesInYPlaneShouldReturnPIOverTwo
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test90DegreesInYPlaneShouldReturnPIOverTwo
)
add_test(
		NAME TestVectorAngle135DegreesInYPlaneShouldReturnThreePIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test135DegreesInYPlaneShouldReturnThreePIOverFour
)
add_test(
		NAME TestVectorAngle225DegreesInYPlaneShouldReturnThreePIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test225DegreesInYPlaneShouldReturnThreePIOverFour
)
add_test(
		NAME TestVectorAngle270DegreesInYPlaneShouldReturnPIOverTwo
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test270DegreesInYPlaneShouldReturnPIOverTwo
)
add_test(
		NAME TestVectorAngle315DegreesInYPlaneShouldReturnPIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test315DegreesInYPlaneShouldReturnPIOverFour
)
add_test(
		NAME TestVectorAngle45DegreesInXPlaneShouldReturnPIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test45DegreesInXPlaneShouldReturnPIOverFour
)
add_test(
		NAME TestVectorAngle135DegreesInXPlaneShouldReturnThreePIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test135DegreesInXPlaneShouldReturnThreePIOverFour
)
add_test(
		NAME TestVectorAngle180DegreesInXPlaneShouldReturnPI
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test180DegreesInXPlaneShouldReturnPI
)
add_test(
		NAME TestVectorAngle225DegreesInXPlaneShouldReturnThreePIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test225DegreesInXPlaneShouldReturnThreePIOverFour
)
add_test(
		NAME TestVectorAngle315DegreesInXPlaneShouldReturnPIOverFour
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test315DegreesInXPlaneShouldReturnPIOverFour
)
add_test(
		NAME TestVectorAngle360DegreesInXPlaneShouldReturnZero
		COMMAND testRoSy --gtest_filter=TestAngleBetweenVectors.Test360DegreesInXPlaneShouldReturnZero
)
add_test(
		NAME TestVectorRotationZeroLengthOShouldThrow
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ZeroLengthOShouldThrow
)
add_test(
		NAME TestVectorRotationNonUnitNormalShouldThrow
		COMMAND testRoSy --gtest_filter=TestVectorRotation.NonUnitNormalShouldThrow
)
add_test(
		NAME TestVectorRotationRotate_2_1_0_about_0_0_1_by_0
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_1_0_about_0_0_1_by_0
)
add_test(
		NAME TestVectorRotationRotate_2_1_0_about_0_0_1_by_1
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_1_0_about_0_0_1_by_1
)
add_test(
		NAME TestVectorRotationRotate_2_1_0_about_0_0_1_by_2
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_1_0_about_0_0_1_by_2
)
add_test(
		NAME TestVectorRotationRotate_2_1_0_about_0_0_1_by_3
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_1_0_about_0_0_1_by_3
)
add_test(
		NAME TestVectorRotationRotate_2_0_1_about_0_1_0_by_0
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_0_1_about_0_1_0_by_0
)
add_test(
		NAME TestVectorRotationRotate_2_0_1_about_0_1_0_by_1
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_0_1_about_0_1_0_by_1
)
add_test(
		NAME TestVectorRotationRotate_2_0_1_about_0_1_0_by_2
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_0_1_about_0_1_0_by_2
)
add_test(
		NAME TestVectorRotationRotate_2_0_1_about_0_1_0_by_3
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_2_0_1_about_0_1_0_by_3
)
add_test(
		NAME TestVectorRotationRotate_0_1_2_about_1_0_0_by_0
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_0_1_2_about_1_0_0_by_0
)
add_test(
		NAME TestVectorRotationRotate_0_1_2_about_1_0_0_by_1
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_0_1_2_about_1_0_0_by_1
)
add_test(
		NAME TestVectorRotationRotate_0_1_2_about_1_0_0_by_2
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_0_1_2_about_1_0_0_by_2
)
add_test(
		NAME TestVectorRotationRotate_0_1_2_about_1_0_0_by_3
		COMMAND testRoSy --gtest_filter=TestVectorRotation.Rotate_0_1_2_about_1_0_0_by_3
)
add_test(
		NAME TestMinimiseKLShouldThrowIfN1IsNotUnitVector
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldThrowIfN1IsNotUnitVector
)
add_test(
		NAME TestMinimiseKLShouldThrowIfO1IsZero
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldThrowIfO1IsZero
)
add_test(
		NAME TestMinimiseKLShouldThrowIfN2IsNotUnitVector
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldThrowIfN2IsNotUnitVector
)
add_test(
		NAME TestMinimiseKLShouldThrowIfO2IsZero
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldThrowIfO2IsZero
)
add_test(
		NAME TestMinimiseKLShouldBe_0_0_For_0_DegreesCoplanar
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldBe_0_0_For_0_DegreesCoplanar
)
add_test(
		NAME TestMinimiseKLShouldBe_0_0_For_30_DegreesCoplanar
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldBe_0_0_For_30_DegreesCoplanar
)
add_test(
		NAME TestMinimiseKLShouldBe_0_0_For_45_DegreesCoplanar
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldBe_0_0_For_45_DegreesCoplanar
)
add_test(
		NAME TestMinimiseKLShouldBe_0_1_For_60_DegreesCoplanar
		COMMAND testRoSy --gtest_filter=TestVectorRotation.ShouldBe_0_1_For_60_DegreesCoplanar
)


# Stash it
install(
		TARGETS testRoSy
		DESTINATION bin
)

