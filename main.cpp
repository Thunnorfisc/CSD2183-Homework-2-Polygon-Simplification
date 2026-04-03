#include <iostream>
#include <fstream>
#include <vector>
#include <utility>

#include "polygon.h"
#include "placement.h"
#include "simplify.h"
#include "symmetric_diff.h"

using Pt = std::pair<double, double>;

// Extract vertices from a ring into a vector of (x,y) pairs
static std::vector<Pt> extract_vertices(const Ring& ring)
{
    std::vector<Pt> verts;
    if (ring.vertices.head == nullptr) return verts;

    Node* cur = ring.vertices.head;
    do {
        verts.push_back({cur->x, cur->y});
        cur = cur->next;
    } while (cur != ring.vertices.head);

    return verts;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: ./simplify <input_file.csv> <target_vertices>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    int target_vertices = std::stoi(argv[2]);

    // Parse CSV
    std::vector<Ring> rings = parse_csv(input_file);

    // Compute input signed areas and store original vertices
    double total_input_area = 0.0;
    int total_vertex_count = 0;
    std::vector<std::vector<Pt>> original_vertices;

    for (auto& ring : rings) {
        ring.original_area = compute_signed_area(ring);
        total_input_area += ring.original_area;
        total_vertex_count += ring.vertices.size;
        original_vertices.push_back(extract_vertices(ring));
    }

    std::cerr << "Parsed " << rings.size() << " rings, "
              << total_vertex_count << " total vertices" << std::endl;

    // Early exit
    if (total_vertex_count <= target_vertices) {
        print_output(rings, total_input_area, total_input_area, 0.0);
        return 0;
    }

    // Simplify (returns cumulative displacement for reference)
    double cumulative_displacement = simplify_polygon(rings, target_vertices);

    // Compute output area
    double total_output_area = 0.0;
    for (auto& ring : rings) {
        total_output_area += compute_signed_area(ring);
    }

    // Compute actual symmetric difference per ring
    double total_sym_diff = 0.0;
    for (size_t i = 0; i < rings.size(); i++) {
        std::vector<Pt> simplified_verts = extract_vertices(rings[i]);
        double ring_sym_diff = compute_symmetric_difference(original_vertices[i], simplified_verts);
        total_sym_diff += ring_sym_diff;
        std::cerr << "  Ring " << rings[i].ring_id
                  << ": symmetric diff = " << ring_sym_diff << std::endl;
    }

    std::cerr << "Input area:  " << total_input_area << std::endl;
    std::cerr << "Output area: " << total_output_area << std::endl;
    std::cerr << "Cumulative displacement: " << cumulative_displacement << std::endl;
    std::cerr << "Symmetric difference:    " << total_sym_diff << std::endl;

    // Use symmetric difference as the reported displacement
    print_output(rings, total_input_area, total_output_area, total_sym_diff);

    return 0;
}