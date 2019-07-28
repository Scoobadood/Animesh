#include "TestDepthMapPyramid.h"
#include "../../mesher/types.h"


void TestDepthMapPyramid::SetUp( ) {}
void TestDepthMapPyramid::TearDown() {}

void
expect_pixel_matches(const DepthMapPyramid::PixelCoord &pixel, unsigned int row, unsigned int col) {
    EXPECT_EQ(pixel.row, row);
    EXPECT_EQ(pixel.col, col);
}

template <typename T, typename ...Ts>
void
expect_contains_exactly(const std::vector<T>& container, const Ts&... items) {
    EXPECT_EQ(sizeof...(items), container.size());

    T values[] = { items... };
    for (auto v : values) {
        EXPECT_TRUE( std::find(container.begin(), container.end(), v) != container.end() );
    }
}

/*
 * *****************************************************************************
 * **            Test Hierarchy
 * *****************************************************************************
 */

TEST_F(TestDepthMapPyramid, AddALevelShouldWork) {
    // 4x4 map with all depths at 10
    DepthMap d{"depthmap_test_data/4x4x10.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);

    EXPECT_EQ(dmp.num_levels(), 2);
}

TEST_F(TestDepthMapPyramid, AddThenRemoveALevelShouldWork) {
    // 4x4 map with all depths at 10
    DepthMap d{"depthmap_test_data/4x4x10.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    dmp.set_num_levels(1);

    EXPECT_EQ(dmp.num_levels(), 1);
}

TEST_F(TestDepthMapPyramid, SetLevelToZeroShouldFail) {
    DepthMap d{"depthmap_test_data/4x4x10.dat"};
    DepthMapPyramid dmp{d};

    try {
        dmp.set_num_levels(0);
        FAIL() << "Expected std::runtime_error";
    }
    catch ( std::runtime_error const & err ){
        EXPECT_EQ( err.what(), std::string( "Can't have less than one level for a DepthMapPyramid") );
    }
    catch( ... ) {
        FAIL( ) <<"Expected std::runtime_error";
    }
}

TEST_F(TestDepthMapPyramid, MergeIdenticalDepthsShouldGiveMean) {
    DepthMap d{"depthmap_test_data/4x4x10.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    EXPECT_EQ( dmp.level(1).depth_at(0,0), 10);
}

TEST_F(TestDepthMapPyramid, MergedLevelShouldBeHalfSize) {
    DepthMap d{"depthmap_test_data/4x4x10.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    EXPECT_EQ( dmp.level(1).rows(), 2 );
    EXPECT_EQ( dmp.level(1).cols(), 2 );
}

TEST_F(TestDepthMapPyramid, MergedDepthsSHouldNotinlcudeZeroes) {
    DepthMap d{"depthmap_test_data/4x4x10_with_zeroes.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    EXPECT_EQ( dmp.level(1).depth_at(0,0), 10);
    EXPECT_EQ( dmp.level(1).depth_at(0,1), 10);
}

TEST_F(TestDepthMapPyramid, MergedDepthsSHouldAggregateZeroesToZero) {
    DepthMap d{"depthmap_test_data/4x4x10_with_zeroes.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    EXPECT_EQ( dmp.level(1).depth_at(1,0), 0);
}

TEST_F(TestDepthMapPyramid, Pixel00ShoudMapCorrectly) {

    DepthMap d{"depthmap_test_data/4x4x10_with_zeroes.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    std::vector<DepthMapPyramid::PixelCoord> mapped_pixels = dmp.mapped_pixels(1, 0, 0);

    expect_contains_exactly(mapped_pixels, DepthMapPyramid::PixelCoord{1,0}, DepthMapPyramid::PixelCoord{1,1}, DepthMapPyramid::PixelCoord{0,1});
}

TEST_F(TestDepthMapPyramid, Pixel01ShoudMapCorrectly) {
    DepthMap d{"depthmap_test_data/4x4x10_with_zeroes.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    std::vector<DepthMapPyramid::PixelCoord> mapped_pixels = dmp.mapped_pixels(1, 0, 1);

    expect_contains_exactly(mapped_pixels,
            DepthMapPyramid::PixelCoord{0,2},
            DepthMapPyramid::PixelCoord{0,3},
            DepthMapPyramid::PixelCoord{1,2},
            DepthMapPyramid::PixelCoord{1,3}
            );
}

TEST_F(TestDepthMapPyramid, Pixel10ShoudMapCorrectly) {
    DepthMap d{"depthmap_test_data/4x4x10_with_zeroes.dat"};
    DepthMapPyramid dmp{d};

    dmp.set_num_levels(2);
    std::vector<DepthMapPyramid::PixelCoord> mapped_pixels = dmp.mapped_pixels(1, 1, 0);

    EXPECT_EQ( mapped_pixels.size(), 0 );
}






