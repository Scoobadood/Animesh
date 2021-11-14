//
// Created by Dave Durbin (Old) on 25/10/21.
//

#include "Optimiser.h"

#include <utility>

Optimiser::Optimiser(Properties properties, std::default_random_engine& rng) //
 : m_properties{ std::move(properties)} //
 , m_random_engine{ rng}
{

}
