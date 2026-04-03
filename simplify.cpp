#include "simplify.h"
#include "heap.h"
#include "topology.h"
#include "spatial_index.h"
#include <iostream>

double simplify_polygon(std::vector<Ring>& rings, int target_vertices)
{
    int total_vertices = 0;
    for (auto& ring : rings) {
        total_vertices += ring.vertices.size;
    }

    if (total_vertices <= target_vertices) return 0.0;

    // ── Build spatial grid ──
    SpatialGrid grid;
    grid.build(rings);

    // ── Build the min-heap with all initial candidates ──
    MinHeap heap;

    for (auto& ring : rings) {
        if (ring.vertices.size < 4) continue;

        Node* start = ring.vertices.head;
        Node* A = start;
        do {
            Node* B = A->next;
            Node* C = B->next;
            Node* D = C->next;
            heap.push(compute_candidate(A, B, C, D));
            A = A->next;
        } while (A != start);
    }

    double total_displacement = 0.0;

    while (total_vertices > target_vertices && !heap.empty()) {
        CollapseCandidate cand = heap.pop();

        if (!cand.is_valid()) continue;

        // Find the ring
        Ring* target_ring = nullptr;
        for (auto& ring : rings) {
            if (ring.ring_id == cand.ring_id) {
                target_ring = &ring;
                break;
            }
        }
        if (!target_ring || target_ring->vertices.size <= 3) continue;

        // ── Topology check using spatial grid ──
        if (collapse_causes_intersection(cand.A, cand.ex, cand.ey, cand.D,
                                         cand.B, cand.C, grid)) {
            continue;
        }

        // ── Perform the collapse ──
        Node* A = cand.A;
        Node* B = cand.B;
        Node* C = cand.C;
        Node* D = cand.D;

        // Remove old segments from grid: A→B, B→C, C→D
        grid.remove(A, B);
        grid.remove(B, C);
        grid.remove(C, D);

        // Insert E into linked list
        Node* E = target_ring->vertices.insert_after(A, cand.ex, cand.ey, cand.ring_id);

        // Remove B and C from linked list
        target_ring->vertices.remove(B);
        target_ring->vertices.remove(C);

        // Insert new segments into grid: A→E, E→D
        grid.insert(A, E);
        grid.insert(E, D);

        // Update versions
        A->version++;
        D->version++;

        total_vertices--;
        total_displacement += cand.displacement;

        // ── Generate new candidates ──
        // After collapse: ...→PP→P→A→E→D→Q→QQ→...
        // All 6 sequences involving A or D:
        if (target_ring->vertices.size >= 4) {
            Node* P = A->prev;
            Node* Q = D->next;

            heap.push(compute_candidate(P, A, E, D));
            heap.push(compute_candidate(A, E, D, Q));

            if (target_ring->vertices.size >= 5) {
                Node* PP = P->prev;
                Node* QQ = Q->next;

                heap.push(compute_candidate(PP, P, A, E));
                heap.push(compute_candidate(E, D, Q, QQ));

                if (target_ring->vertices.size >= 6) {
                    Node* PPP = PP->prev;
                    Node* QQQ = QQ->next;

                    heap.push(compute_candidate(PPP, PP, P, A));
                    heap.push(compute_candidate(D, Q, QQ, QQQ));
                }
            }
        }
    }

    std::cerr << "Simplification done. Total displacement: " << total_displacement << std::endl;
    std::cerr << "Vertices remaining: " << total_vertices << std::endl;

    return total_displacement;
}