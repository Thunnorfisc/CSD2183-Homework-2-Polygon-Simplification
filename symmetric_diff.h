#pragma once
#include <vector>
#include <utility>

// Compute the area of the symmetric difference between two simple polygons
// Both polygons should have the same signed area (area-preserving simplification)
// Vertices should be in order (CCW for exterior, CW for interior)
double compute_symmetric_difference(
    const std::vector<std::pair<double, double>>& original,
    const std::vector<std::pair<double, double>>& simplified);