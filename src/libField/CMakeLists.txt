file(GLOB SOURCES "src/*.cpp")

add_library(Field STATIC ${SOURCES})

# Need Eigen3
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

# PCL
find_package(PCL 1.3 REQUIRED)
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

target_include_directories(
	Field PUBLIC 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	${EIGEN3_INCLUDE_DIR}
	${PCL_INCLUDE_DIRS}
	PRIVATE src)

target_link_libraries( Field 
	FileUtils
	RoSy
	Graph
	${PCL_COMMON_LIBRARIES}
	${PCL_FEATURES_LIBRARIES}
	${PCL_IO_LIBRARIES} )

# This makes the project importable from the build directory
export(
	TARGETS Field
	FILE FieldLibraryConfig.cmake)

# Tests
file( GLOB TEST_SOURCES "tests/*.cpp" )

add_executable( testField
    ${TEST_SOURCES})

target_link_libraries( testField
	Field
	Graph 
	gtest 
	gmock
	${CMAKE_THREAD_LIBS_INIT} )

add_test( NAME TestFieldDataTangentVectorIsPerpendicularToNormal COMMAND testField --gtest_filter=TestFieldData.TangentVectorIsPerpendicularToNormal)
add_test( NAME TestFieldDataTangentVectorIsUnit COMMAND testField --gtest_filter=TestFieldData.TangentVectorIsUnit )
add_test( NAME TestFieldDataKIsZero COMMAND testField --gtest_filter=TestFieldData.KIsZero  )

# FieldElement Tests
add_test( NAME TestFieldElement_Pass COMMAND testField --gtest_filter=TestFieldElement.passingTest  )
add_test( NAME TestFieldElement_twoArgConstructWithNonUnitNormalShouldThrow COMMAND testField --gtest_filter=TestFieldElement.twoArgConstructWithNonUnitNormalShouldThrow  )
add_test( NAME TestFieldElement.twoArgConstructWithUnitNormalShouldGenerateUnitTangent COMMAND testField --gtest_filter=TestFieldElement.twoArgConstructWithUnitNormalShouldGenerateUnitTangent )
add_test( NAME TestFieldElement.twoArgConstructWithUnitNormalShouldGeneratePerpTangent COMMAND testField --gtest_filter=TestFieldElement.twoArgConstructWithUnitNormalShouldGeneratePerpTangent )
add_test( NAME TestFieldElement.threeArgConstructWithNonUnitNormalShouldThrow COMMAND testField --gtest_filter=TestFieldElement.threeArgConstructWithNonUnitNormalShouldThrow )
add_test( NAME TestFieldElement.threeArgConstructWithNonUnitTangentShouldThrow COMMAND testField --gtest_filter=TestFieldElement.threeArgConstructWithNonUnitTangentShouldThrow )
add_test( NAME TestFieldElement.threeArgConstructWithNonPerpTangentShouldThrow COMMAND testField --gtest_filter=TestFieldElement.threeArgConstructWithNonPerpTangentShouldThrow )
add_test( NAME TestFieldElement.mergeIdenticalVectorsShouldGiveSameFE COMMAND testField --gtest_filter=TestFieldElement.mergeIdenticalVectorsShouldGiveSameFE )
add_test( NAME TestFieldElement.mergeVectorsWithCommonNormalShouldHaveSameNormal COMMAND testField --gtest_filter=TestFieldElement.mergeVectorsWithCommonNormalShouldHaveSameNormal )


# add_test( NAME  COMMAND testField --gtest_filter=  )


# Stash it
install(TARGETS testField DESTINATION bin)

