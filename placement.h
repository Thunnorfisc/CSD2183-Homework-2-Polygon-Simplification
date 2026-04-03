#pragma once
#include "polygon.h"
#include <cstdint>

struct CollapseCandidate {
    Node* A;
    Node* B;  // will be removed
    Node* C;  // will be removed
    Node* D;
    double ex, ey;        // Steiner point position
    double displacement;  // areal displacement cost
    int ring_id;
    uint64_t serial;      // tie-breaking: lower serial = earlier candidate

    // Validity check: verify linked list structure is still intact
    bool is_valid() const {
        return A && B && C && D
            && !A->deleted && !B->deleted && !C->deleted && !D->deleted
            && A->next == B && B->next == C && C->next == D
            && A->ring_id == B->ring_id && B->ring_id == C->ring_id
            && C->ring_id == D->ring_id;
    }
};

struct CompareCandidates {
    bool operator()(const CollapseCandidate& a, const CollapseCandidate& b) const {
        // Compare by displacement first, then by serial for tie-breaking
        double diff = a.displacement - b.displacement;
        if (diff > 1e-9) return true;    // a has larger displacement = lower priority
        if (diff < -1e-9) return false;   // a has smaller displacement = higher priority
        return a.serial > b.serial;       // earlier serial = higher priority
    }
};

// Compute the best candidate collapse for A→B→C→D
// Tries both AB and CD intersections, picks the one with lower displacement
// Returns false if no valid placement exists
bool compute_candidate(Node* A, Node* B, Node* C, Node* D, CollapseCandidate& out);