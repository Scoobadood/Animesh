#pragma once

#include <gtest/gtest.h>
#include <DepthMap/DepthMap.h>
#include <Surfel/Surfel.h>
#include <Surfel/PixelInFrame.h>

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

    Surfel s1{
            "id1",
            std::vector<FrameData>{
                    FrameData{test_pixel_frame_2, 10.0f, Eigen::Matrix3f::Identity(), Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()},
                    FrameData{test_pixel, 10.0f, Eigen::Matrix3f::Identity(), Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()}
            },
            Eigen::Vector3f{1.0, 0.0, 0.0},
            Eigen::Vector2f{1.0, 0.0}
    };

    Surfel s1_neighbour{
            "id2",
            std::vector<FrameData>{
                    FrameData{test_pixel_up, 10.0f, Eigen::Matrix3f::Identity(), Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()}
            },
            Eigen::Vector3f{1.0, 0.0, 0.0},
            Eigen::Vector2f{1.0, 0.0}
    };
    Surfel s1_not_neighbour{
            "id3",
            std::vector<FrameData>{
                    FrameData{test_pixel_far_away, 10.0f, Eigen::Matrix3f::Identity(), Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()}
            },
            Eigen::Vector3f{1.0, 0.0, 0.0},
            Eigen::Vector2f{1.0, 0.0}
    };

public:
    void SetUp() override;

    void TearDown() override;
};
