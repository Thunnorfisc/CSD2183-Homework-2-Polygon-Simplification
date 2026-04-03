#pragma once
#include "polygon.h"

struct CollapseCandidate {
    Node* A;
    Node* B;  // will be removed
    Node* C;  // will be removed
    Node* D;
    double ex, ey;        // Steiner point position
    double displacement;  // areal displacement cost
    int ring_id;

    // Version stamps for lazy deletion
    uint64_t vA, vB, vC, vD;

    bool is_valid() const {
        return !A->deleted && !B->deleted && !C->deleted && !D->deleted
            && A->version == vA && B->version == vB
            && C->version == vC && D->version == vD;
    }
};

struct CompareCandidates {
    bool operator()(const CollapseCandidate& a, const CollapseCandidate& b) const {
        return a.displacement > b.displacement;
    }
};

CollapseCandidate compute_candidate(Node* A, Node* B, Node* C, Node* D);