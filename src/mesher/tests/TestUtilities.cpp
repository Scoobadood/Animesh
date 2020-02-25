#include "TestUtilities.h"

#include "../surfel_compute.h"

void TestUtilities::SetUp( ) {}
void TestUtilities::TearDown() {}

/*
 * Test pixels in all 8 neighbourhoods are 8 connected under 8 connected ness
 *
 *
 *     1 2 3
 *     4 x 5
 *     6 7 8
 */
TEST_F( TestUtilities, 8_connected_PIFs_1_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up_left, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_2_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_3_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up_right, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_4_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_left, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_5_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_right, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_6_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down_left, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_7_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_8_are_8_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down_right, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_8_copy_is_not_neighbour ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_copy, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_8_wrong_frame_is_not_neighbour ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_frame_2, true));
}
TEST_F( TestUtilities, 8_connected_PIFs_8_far_away_is_not_neighbour ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_far_away, true));
}


TEST_F( TestUtilities, 4_connected_PIFs_1_are_not_neighbours ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_up_left, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_2_are_4_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_3_are_not_neighbours ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_up_right, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_4_are_4_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_left, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_5_are_4_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_right, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_6_are_not_neighbours ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_down_left, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_7_are_4_neighbours ) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_8_are_not_neighbours ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_down_right, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_4_copy_is_not_neighbour ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_copy, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_4_wrong_frame_is_not_neighbour ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_frame_2, false));
}
TEST_F( TestUtilities, 4_connected_PIFs_4_far_away_is_not_neighbour ) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_far_away, false));
}




