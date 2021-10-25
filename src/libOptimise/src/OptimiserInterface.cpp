//
// Created by Dave Durbin (Old) on 25/10/21.
//

#include "OptimiserInterface.h"

#include <utility>

OptimiserInterface::OptimiserInterface(Properties properties, std::mt19937& rng) //
 : m_properties{ std::move(properties)} //
 , m_random_engine{ rng}
{

}
