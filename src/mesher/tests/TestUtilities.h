#pragma once

#include <gtest/gtest.h>
#include <DepthMap/DepthMap.h>
#include "../types.h"

class TestUtilities : public ::testing::Test {
protected:
    PixelInFrame test_pixel{4, 4, 1};
    PixelInFrame test_pixel_up_left{3, 3, 1};
    PixelInFrame test_pixel_up{4, 3, 1};
    PixelInFrame test_pixel_up_right{5, 3, 1};
    PixelInFrame test_pixel_left{3, 4, 1};
    PixelInFrame test_pixel_right{5, 4, 1};
    PixelInFrame test_pixel_down_left{3, 5, 1};
    PixelInFrame test_pixel_down{4, 5, 1};
    PixelInFrame test_pixel_down_right{5, 5, 1};

    PixelInFrame test_pixel_copy{4, 4, 1};
    PixelInFrame test_pixel_frame_2{4, 4, 2};
    PixelInFrame test_pixel_far_away{8, 9, 1};

public:
	void SetUp( );
	void TearDown();
};
