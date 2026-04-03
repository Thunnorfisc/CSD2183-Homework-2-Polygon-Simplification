#pragma once
#include "placement.h"
#include <vector>

class MinHeap {
private:
    std::vector<CollapseCandidate> data;

    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return 2 * i + 1; }
    int right(int i) { return 2 * i + 2; }

    void sift_up(int i);
    void sift_down(int i);

public:
    void push(const CollapseCandidate& cand);

    CollapseCandidate pop();

    const CollapseCandidate& top() const;

    bool empty() const { return data.empty(); }
    int size() const { return static_cast<int>(data.size()); }
};