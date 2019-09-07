#pragma once

#include <vector>
#include "surfel_compute.h"

/**
 * Perform orientation field optimisation.
 * Continuously step until done.
 */
void
optimise(std::vector<Surfel>& surfels);