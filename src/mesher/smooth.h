#pragma once

#include <vector>
#include "surfel_compute.hpp"

/**
 * Perform orientation field optimisation.
 * Continuously step until done.
 */
void
optimise(std::vector<Surfel>& surfels);