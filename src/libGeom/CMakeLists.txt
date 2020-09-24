add_library(
		Geom SHARED
		include/Geom/Checks.h src/Checks.cpp
		include/Geom/Geom.h src/Geom.cpp
)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
		Geom
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		${EIGEN3_INCLUDE_DIR}
		PRIVATE
		include/Geom)

add_executable(
		testGeom
		tests/main.cpp
		tests/TestGeom.cpp tests/TestGeom.h
)

target_link_libraries(
		testGeom
		Geom
		gtest
		gmock
)

add_test(
		NAME SkewSymmetricMatrixShouldBeCorrect
		COMMAND testGeom --gtest_filter=SkewSymmetricMatrixShouldBeCorrect
)
add_test(
		NAME VectorPerpendicularToZeroShouldThrow
		COMMAND testGeom --gtest_filter=VectorPerpendicularToZeroShouldThrow
)
add_test(
		NAME VectorPerpendicularTo_1_0_0_is_perpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularTo_1_0_0_is_perpendicular
)
add_test(
		NAME VectorPerpendicularTo_0_1_0_is_perpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularTo_0_1_0_is_perpendicular
)
add_test(
		NAME VectorPerpendicularTo_0_0_1_is_perpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularTo_0_0_1_is_perpendicular
)
add_test(
		NAME VectorPerpendicularTo_1_1_0_is_perpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularTo_1_1_0_is_perpendicular
)
add_test(
		NAME VectorPerpendicularTo_1_0_1_is_perpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularTo_1_0_1_is_perpendicular
)
add_test(
		NAME VectorPerpendicularTo_0_1_1_is_perpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularTo_0_1_1_is_perpendicular
)
add_test(
		NAME VectorPerpendicularTo_1_1_1_is_perpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularTo_1_1_1_is_perpendicular
)
add_test(
		NAME VectorPerpendicularToManyIsActuallyPerpendicular
		COMMAND testGeom --gtest_filter=VectorPerpendicularToManyIsActuallyPerpendicular
)
add_test(
		NAME Vector2VectorShouldThrowIfFirstVector0
		COMMAND testGeom --gtest_filter=Vector2VectorShouldThrowIfFirstVector0
)
add_test(
		NAME Vector2VectorShouldThrowIfSecondVector0
		COMMAND testGeom --gtest_filter=Vector2VectorShouldThrowIfSecondVector0
)
add_test(
		NAME RotateVectorsAlignXToY
		COMMAND testGeom --gtest_filter=RotateVectorsAlignXToY
)
add_test(
		NAME RotateVectorsAlignXToZ
		COMMAND testGeom --gtest_filter=RotateVectorsAlignXToZ
)
add_test(
		NAME RotateVectorsAlignYToZ
		COMMAND testGeom --gtest_filter=RotateVectorsAlignYToZ
)
add_test(
		NAME RotateVectorsAlignZToY
		COMMAND testGeom --gtest_filter=RotateVectorsAlignZToY
)
add_test(
		NAME RotateVectorsAlignZToX
		COMMAND testGeom --gtest_filter=RotateVectorsAlignZToX
)
add_test(
		NAME RotateVectorsAlignYToX
		COMMAND testGeom --gtest_filter=RotateVectorsAlignYToX
)
add_test(
		NAME RotateVectorsAlignRandomToRandom
		COMMAND testGeom --gtest_filter=RotateVectorsAlignRandomToRandom
)
add_test(
		NAME RotateSameVectorsShouldReturnIdentity
		COMMAND testGeom --gtest_filter=RotateSameVectorsShouldReturnIdentity
)
add_test(
		NAME RotateOpposingVectorsShouldReturn
		COMMAND testGeom --gtest_filter=RotateOpposingVectorsShouldReturn
)
add_test(
		NAME Align_P_Zero_N_Z_R_none
		COMMAND testGeom --gtest_filter=Align_P_Zero_N_Z_R_none
)
add_test(
		NAME Align_P_Zero_N_1_1_1_R_none
		COMMAND testGeom --gtest_filter=Align_P_Zero_N_1_1_1_R_none
)
add_test(
		NAME Align_P_Zero_N_1_1_1_R_20
		COMMAND testGeom --gtest_filter=Align_P_Zero_N_1_1_1_R_20
)
add_test(
		NAME Align_P_Zero_N_1_1_1_R_30
		COMMAND testGeom --gtest_filter=Align_P_Zero_N_1_1_1_R_30
)
add_test(
		NAME Align_P_Zero_N_1_1_1_R_40
		COMMAND testGeom --gtest_filter=Align_P_Zero_N_1_1_1_R_40
)
add_test(
		NAME Align_P_Zero_N_1_1_1_R_20_30_40
		COMMAND testGeom --gtest_filter=Align_P_Zero_N_1_1_1_R_20_30_40
)
add_test(
		NAME Align_P_XY_N_Z_R_none
		COMMAND testGeom --gtest_filter=Align_P_XY_N_Z_R_none
)
add_test(
		NAME Align_P_XY_N_1_1_1_R_none
		COMMAND testGeom --gtest_filter=Align_P_XY_N_1_1_1_R_none
)
add_test(
		NAME Align_P_XY_N_1_1_1_R_20
		COMMAND testGeom --gtest_filter=Align_P_XY_N_1_1_1_R_20
)
add_test(
		NAME Align_P_XY_N_1_1_1_R_30
		COMMAND testGeom --gtest_filter=Align_P_XY_N_1_1_1_R_30
)
add_test(
		NAME Align_P_XY_N_1_1_1_R_40
		COMMAND testGeom --gtest_filter=Align_P_XY_N_1_1_1_R_40
)
add_test(
		NAME Align_P_XY_N_1_1_1_R_20_30_40
		COMMAND testGeom --gtest_filter=Align_P_XY_N_1_1_1_R_20_30_40
)
add_test(
		NAME Align_P_3D_N_Z_R_none
		COMMAND testGeom --gtest_filter=Align_P_3D_N_Z_R_none
)
add_test(
		NAME Align_P_3D_N_1_1_1_R_none
		COMMAND testGeom --gtest_filter=Align_P_3D_N_1_1_1_R_none
)
add_test(
		NAME Align_P_3D_N_1_1_1_R_20
		COMMAND testGeom --gtest_filter=Align_P_3D_N_1_1_1_R_20
)
add_test(
		NAME Align_P_3D_N_1_1_1_R_30
		COMMAND testGeom --gtest_filter=Align_P_3D_N_1_1_1_R_30
)
add_test(
		NAME Align_P_3D_N_1_1_1_R_40
		COMMAND testGeom --gtest_filter=Align_P_3D_N_1_1_1_R_40
)
add_test(
		NAME Align_P_3D_N_1_1_1_R_20_30_40
		COMMAND testGeom --gtest_filter=Align_P_3D_N_1_1_1_R_20_30_40
)
add_test(
		NAME Align_P_XYC_N_Z_R_none
		COMMAND testGeom --gtest_filter=Align_P_XYC_N_Z_R_none
)
add_test(
		NAME Align_P_XYC_N_1_1_1_R_none
		COMMAND testGeom --gtest_filter=Align_P_XYC_N_1_1_1_R_none
)
add_test(
		NAME Align_P_XYC_N_1_1_1_R_20
		COMMAND testGeom --gtest_filter=Align_P_XYC_N_1_1_1_R_20
)
add_test(
		NAME Align_P_XYC_N_1_1_1_R_30
		COMMAND testGeom --gtest_filter=Align_P_XYC_N_1_1_1_R_30
)
add_test(
		NAME Align_P_XYC_N_1_1_1_R_40
		COMMAND testGeom --gtest_filter=Align_P_XYC_N_1_1_1_R_40
)
add_test(
		NAME Align_P_XYC_N_1_1_1_R_20_30_40
		COMMAND testGeom --gtest_filter=Align_P_XYC_N_1_1_1_R_20_30_40
)
add_test(
		NAME Align_P_3DC_N_Z_R_none
		COMMAND testGeom --gtest_filter=Align_P_3DC_N_Z_R_none
)
add_test(
		NAME Align_P_3DC_N_1_1_1_R_none
		COMMAND testGeom --gtest_filter=Align_P_3DC_N_1_1_1_R_none
)
add_test(
		NAME Align_P_3DC_N_1_1_1_R_20
		COMMAND testGeom --gtest_filter=Align_P_3DC_N_1_1_1_R_20
)
add_test(
		NAME Align_P_3DC_N_1_1_1_R_30
		COMMAND testGeom --gtest_filter=Align_P_3DC_N_1_1_1_R_30
)
add_test(
		NAME Align_P_3DC_N_1_1_1_R_40
		COMMAND testGeom --gtest_filter=Align_P_3DC_N_1_1_1_R_40
)
add_test(
		NAME Align_P_3DC_N_1_1_1_R_20_30_40
		COMMAND testGeom --gtest_filter=Align_P_3DC_N_1_1_1_R_20_30_40
)
add_test(
		NAME Align_P_3D_N_Z_R_20
		COMMAND testGeom --gtest_filter=Align_P_3D_N_Z_R_20
)
add_test(
		NAME Align_P_3D_N_Z_R_30
		COMMAND testGeom --gtest_filter=Align_P_3D_N_Z_R_30
)
add_test(
		NAME Align_P_3D_N_Z_R_40
		COMMAND testGeom --gtest_filter=Align_P_3D_N_Z_R_40
)
add_test(
		NAME Align_P_3D_N_Z_R_20_30_40
		COMMAND testGeom --gtest_filter=Align_P_3D_N_Z_R_20_30_40
)

# Stash it
install(
		TARGETS testGeom
		DESTINATION bin
)