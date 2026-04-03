#pragma once
#include "polygon.h"
#include "spatial_index.h"
#include <vector>

bool segments_intersect(double p1x, double p1y, double p2x, double p2y,
                        double p3x, double p3y, double p4x, double p4y);

// Fast version using spatial index
bool collapse_causes_intersection(Node* A, double ex, double ey, Node* D,
                                  Node* B, Node* C,
                                  const SpatialGrid& grid);