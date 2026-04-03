#pragma once
#include "polygon.h"
#include "placement.h"
#include <vector>

double simplify_polygon(std::vector<Ring>& rings, int target_vertices);