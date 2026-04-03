#pragma once
#include "polygon.h"
#include <vector>
#include <unordered_set>

// A segment in the spatial index, defined by two node endpoints
struct Segment {
    Node* from;
    Node* to;

    bool operator==(const Segment& other) const {
        return from == other.from && to == other.to;
    }
};

// Hash for Segment so we can use unordered_set
struct SegmentHash {
    size_t operator()(const Segment& s) const {
        auto h1 = std::hash<Node*>{}(s.from);
        auto h2 = std::hash<Node*>{}(s.to);
        return h1 ^ (h2 << 1);
    }
};

class SpatialGrid {
private:
    double min_x, min_y, max_x, max_y;
    double cell_width, cell_height;
    int cols, rows;

    // Each cell stores a set of segments that overlap it
    std::vector<std::unordered_set<Segment, SegmentHash>> cells;

    // Convert world coordinate to grid cell index
    int cell_x(double x) const;
    int cell_y(double y) const;

    // Get all cell indices that a bounding box overlaps
    void get_cells(double x1, double y1, double x2, double y2,
                   int& c0, int& r0, int& c1, int& r1) const;

public:
    // Build the grid from all rings
    void build(const std::vector<Ring>& rings);

    // Insert a segment into the grid
    void insert(Node* from, Node* to);

    // Remove a segment from the grid
    void remove(Node* from, Node* to);

    // Query: find all segments whose bounding box overlaps the query bbox
    // Returns them in 'result'. Excludes segments in 'exclude'.
    void query(double qx1, double qy1, double qx2, double qy2,
               std::vector<Segment>& result,
               const std::unordered_set<Segment, SegmentHash>& exclude) const;
};