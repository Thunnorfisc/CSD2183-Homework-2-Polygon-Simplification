#include "heap.h"
#include <stdexcept>
#include <utility>
#include <cmath>

bool MinHeap::higher_priority(const CollapseCandidate& a, const CollapseCandidate& b)
{
    double diff = a.displacement - b.displacement;
    if (diff < -1e-9) return true;   // a is smaller = higher priority
    if (diff > 1e-9) return false;   // a is larger = lower priority
    return a.serial < b.serial;       // tie-break: earlier serial wins
}

void MinHeap::sift_up(int i)
{
    while (i > 0) {
        int p = parent(i);
        if (higher_priority(data[i], data[p])) {
            std::swap(data[i], data[p]);
            i = p;
        } else {
            break;
        }
    }
}

void MinHeap::sift_down(int i)
{
    int n = static_cast<int>(data.size());

    while (true) {
        int best = i;
        int l = left(i);
        int r = right(i);

        if (l < n && higher_priority(data[l], data[best])) {
            best = l;
        }
        if (r < n && higher_priority(data[r], data[best])) {
            best = r;
        }

        if (best != i) {
            std::swap(data[i], data[best]);
            i = best;
        } else {
            break;
        }
    }
}

void MinHeap::push(const CollapseCandidate& cand)
{
    data.push_back(cand);
    sift_up(static_cast<int>(data.size()) - 1);
}

CollapseCandidate MinHeap::pop()
{
    if (data.empty()) {
        throw std::runtime_error("MinHeap::pop() called on empty heap");
    }

    CollapseCandidate result = data[0];
    data[0] = data.back();
    data.pop_back();

    if (!data.empty()) {
        sift_down(0);
    }

    return result;
}

const CollapseCandidate& MinHeap::top() const
{
    if (data.empty()) {
        throw std::runtime_error("MinHeap::top() called on empty heap");
    }
    return data[0];
}