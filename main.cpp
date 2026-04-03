// DSA HW2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>

#include "polygon.h"
#include "placement.h"
#include "simplify.h"
/*
main(argc, argv)
│
├── 1. Parse command-line args
│       → input_file path
│       → target_vertices (integer n)
│
├── 2. Parse CSV
│       → Build circular doubly-linked list for each ring
│       → Store rings in a vector<Ring>
│
├── 3. Compute input signed areas
│       → For each ring, run shoelace on its linked list
│       → Sum them all for total input signed area
│
├── 4. Check early exit
│       → If total vertex count <= n, just print input as-is and stop
│
├── 5. Initialize candidates
│       → For every 4-vertex sequence in every ring,
│         compute placement(A,B,C,D) → E, displacement
│       → Push all candidates into the min-heap
│
├── 6. Main simplification loop
│       while (total_vertex_count > n):
│       │
│       ├── Pop min-displacement candidate from heap
│       ├── If stale (version mismatch), discard, continue
│       ├── If ring would go below 3 vertices, discard, continue
│       ├── Run topology check (AE and ED vs all other segments)
│       │     If intersection found, discard, continue
│       ├── Perform the collapse:
│       │     ├── Create node E, splice into linked list
│       │     ├── Remove B and C from linked list
│       │     ├── Update spatial index (remove old segs, insert new)
│       │     ├── Increment versions on A, D
│       │     ├── Decrement ring vertex count
│       │     ├── Decrement total_vertex_count
│       │     └── Generate new candidates for sequences involving E
│       │           Push them onto heap
│       │
│       └── If heap is empty, break (can't simplify further)
│
├── 7. Compute output signed areas
│       → Shoelace on each ring again
│
├── 8. Compute total areal displacement
│       → You accumulate this during the loop (sum of each
│         collapse's displacement) OR compute the symmetric
│         difference at the end
│
└── 9. Print output
        → Header line
        → Walk each ring's linked list, print vertices
        → Three summary lines (input area, output area, displacement)
*/
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

    // Compute input signed areas
    double total_input_area = 0.0;
    int total_vertex_count = 0;

    for (auto& ring : rings) {
        ring.original_area = compute_signed_area(ring);
        total_input_area += ring.original_area;
        total_vertex_count += ring.vertices.size;
    }

    std::cerr << "Parsed " << rings.size() << " rings, "
              << total_vertex_count << " total vertices" << std::endl;

    // Early exit
    if (total_vertex_count <= target_vertices) {
        print_output(rings, total_input_area, total_input_area, 0.0);
        return 0;
    }

    // Simplify
    double displacement = simplify_polygon(rings, target_vertices);

    // Compute output area
    double total_output_area = 0.0;
    for (auto& ring : rings) {
        total_output_area += compute_signed_area(ring);
    }

    // Compute displacement (recompute from areas for now)
    // TODO: track properly or compute symmetric difference
    // For now use the difference as a sanity check
    std::cerr << "Input area:  " << total_input_area << std::endl;
    std::cerr << "Output area: " << total_output_area << std::endl;

    print_output(rings, total_input_area, total_output_area, displacement);

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
