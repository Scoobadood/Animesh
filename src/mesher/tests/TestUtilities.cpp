#include "TestUtilities.h"

#include <Surfel/Surfel_Compute.h>
#include <memory>

void TestUtilities::SetUp() {

}

void TestUtilities::TearDown() {}

/*
 * Test pixels in all 8 neighbourhoods are 8 connected under 8 connected ness
 *
 *
 *     1 2 3
 *     4 x 5
 *     6 7 8
 */
TEST_F(TestUtilities, 8_connected_PIFs_1_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up_left, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_2_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_3_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up_right, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_4_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_left, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_5_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_right, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_6_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down_left, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_7_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_8_are_8_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down_right, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_8_copy_is_not_neighbour) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_copy, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_8_wrong_frame_is_not_neighbour) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_frame_2, true));
}

TEST_F(TestUtilities, 8_connected_PIFs_8_far_away_is_not_neighbour) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_far_away, true));
}

// Found passing in wilderness
TEST_F(TestUtilities, not_8_connected_diff_x_1_y_gt_1) {
    EXPECT_FALSE(are_neighbours(PixelInFrame{4, 4, 1}, PixelInFrame{5, 10, 1}, true));
}

TEST_F(TestUtilities, not_8_connected_diff_x_gt_1_y_1) {
    EXPECT_FALSE(are_neighbours(PixelInFrame{4, 4, 1}, PixelInFrame{10, 5, 1}, true));
}


TEST_F(TestUtilities, 4_connected_PIFs_1_are_not_neighbours) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_up_left, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_2_are_4_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_up, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_3_are_not_neighbours) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_up_right, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_4_are_4_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_left, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_5_are_4_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_right, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_6_are_not_neighbours) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_down_left, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_7_are_4_neighbours) {
    EXPECT_TRUE(are_neighbours(test_pixel, test_pixel_down, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_8_are_not_neighbours) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_down_right, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_4_copy_is_not_neighbour) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_copy, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_4_wrong_frame_is_not_neighbour) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_frame_2, false));
}

TEST_F(TestUtilities, 4_connected_PIFs_4_far_away_is_not_neighbour) {
    EXPECT_FALSE(are_neighbours(test_pixel, test_pixel_far_away, false));
}

// Test Surfel adjacency
TEST_F(TestUtilities, neighburing_surfels_are_found) {
    EXPECT_TRUE(are_neighbours(
            std::make_shared<Surfel>(s1),
            std::make_shared<Surfel>(s1_neighbour),
            true));
}

TEST_F(TestUtilities, non_neighburing_surfels_are_found) {
    EXPECT_FALSE(are_neighbours(
            std::make_shared<Surfel>(s1),
            std::make_shared<Surfel>(s1_not_neighbour),
            true));
}

TEST_F(TestUtilities, populate_surfel_neighbours) {
    std::vector<std::shared_ptr<Surfel>> surfels{
            std::make_shared<Surfel>(s1),
            std::make_shared<Surfel>(s1_neighbour)};
    auto graph = graph_from_surfels(surfels, true);

    auto n1 = graph.nodes().at(0);
    auto n2 = graph.nodes().at(1);
    auto n = graph.neighbours(n1);
    EXPECT_EQ(n.size(), 1);
    EXPECT_EQ(n.at(0)->data()->id, n2->data()->id);

    n = graph.neighbours(n2);
    EXPECT_EQ(n.size(), 1);
    EXPECT_EQ(n.at(0)->data()->id, n1->data()->id);
}

TEST_F(TestUtilities, fail_to_populate_surfel_neighbours) {

    std::vector<std::shared_ptr<Surfel>> surfels{
            std::make_shared<Surfel>(s1),
            std::make_shared<Surfel>(s1_not_neighbour)};
    auto graph = graph_from_surfels(surfels, true);

    auto n1 = graph.nodes().at(0);
    auto n2 = graph.nodes().at(1);

    EXPECT_EQ(graph.neighbours(n1).size(), 0);
    EXPECT_EQ(graph.neighbours(n2).size(), 0);
}