#include "spatial_index.h"
#include <algorithm>
#include <cmath>

int SpatialGrid::cell_x(double x) const
{
    int c = static_cast<int>((x - min_x) / cell_width);
    return std::max(0, std::min(c, cols - 1));
}

int SpatialGrid::cell_y(double y) const
{
    int r = static_cast<int>((y - min_y) / cell_height);
    return std::max(0, std::min(r, rows - 1));
}

void SpatialGrid::get_cells(double x1, double y1, double x2, double y2,
                            int& c0, int& r0, int& c1, int& r1) const
{
    // Ensure x1 <= x2, y1 <= y2
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    c0 = cell_x(x1);
    r0 = cell_y(y1);
    c1 = cell_x(x2);
    r1 = cell_y(y2);
}

void SpatialGrid::build(const std::vector<Ring>& rings)
{
    // Compute bounding box of all vertices
    min_x = 1e18;  min_y = 1e18;
    max_x = -1e18; max_y = -1e18;

    int total_segments = 0;
    for (const auto& ring : rings) {
        Node* cur = ring.vertices.head;
        do {
            min_x = std::min(min_x, cur->x);
            min_y = std::min(min_y, cur->y);
            max_x = std::max(max_x, cur->x);
            max_y = std::max(max_y, cur->y);
            total_segments++;
            cur = cur->next;
        } while (cur != ring.vertices.head);
    }

    // Add small padding to avoid edge cases
    double pad = 1.0;
    min_x -= pad; min_y -= pad;
    max_x += pad; max_y += pad;

    // Grid size: roughly sqrt(n) x sqrt(n) cells
    int grid_size = std::max(1, static_cast<int>(std::sqrt(total_segments)));
    cols = grid_size;
    rows = grid_size;

    cell_width = (max_x - min_x) / cols;
    cell_height = (max_y - min_y) / rows;

    // Avoid zero-size cells
    if (cell_width < 1e-12) cell_width = 1.0;
    if (cell_height < 1e-12) cell_height = 1.0;

    cells.clear();
    cells.resize(cols * rows);

    // Insert all segments
    for (const auto& ring : rings) {
        Node* cur = ring.vertices.head;
        do {
            insert(cur, cur->next);
            cur = cur->next;
        } while (cur != ring.vertices.head);
    }
}

void SpatialGrid::insert(Node* from, Node* to)
{
    Segment seg{from, to};

    int c0, r0, c1, r1;
    get_cells(from->x, from->y, to->x, to->y, c0, r0, c1, r1);

    for (int r = r0; r <= r1; r++) {
        for (int c = c0; c <= c1; c++) {
            cells[r * cols + c].insert(seg);
        }
    }
}

void SpatialGrid::remove(Node* from, Node* to)
{
    Segment seg{from, to};

    int c0, r0, c1, r1;
    get_cells(from->x, from->y, to->x, to->y, c0, r0, c1, r1);

    for (int r = r0; r <= r1; r++) {
        for (int c = c0; c <= c1; c++) {
            cells[r * cols + c].erase(seg);
        }
    }
}

void SpatialGrid::query(double qx1, double qy1, double qx2, double qy2,
                        std::vector<Segment>& result,
                        const std::unordered_set<Segment, SegmentHash>& exclude) const
{
    result.clear();

    int c0, r0, c1, r1;
    get_cells(qx1, qy1, qx2, qy2, c0, r0, c1, r1);

    // Use a set to deduplicate (segments span multiple cells)
    std::unordered_set<Segment, SegmentHash> seen;

    for (int r = r0; r <= r1; r++) {
        for (int c = c0; c <= c1; c++) {
            for (const auto& seg : cells[r * cols + c]) {
                if (exclude.count(seg) == 0 && seen.insert(seg).second) {
                    result.push_back(seg);
                }
            }
        }
    }
}