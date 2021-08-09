//
// Created by Dave Durbin on 2019-07-06.
//
#pragma once

#ifndef ANIMESH_TYPES_H
#define ANIMESH_TYPES_H

#include <string>
#include <Eigen/Core>



struct NormalTangent {
    Eigen::Vector3f normal;
    Eigen::Vector3f tangent;

    NormalTangent(Eigen::Vector3f n, Eigen::Vector3f t) : normal{std::move(n)}, tangent{std::move(t)} {
    }
};

#endif //ANIMESH_TYPES_H
