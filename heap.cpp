#include "heap.h"
#include <stdexcept>
#include <utility>

void MinHeap::sift_up(int i)
{
    while (i > 0) {
        int p = parent(i);
        if (data[i].displacement < data[p].displacement) {
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
        int smallest = i;
        int l = left(i);
        int r = right(i);

        if (l < n && data[l].displacement < data[smallest].displacement) {
            smallest = l;
        }
        if (r < n && data[r].displacement < data[smallest].displacement) {
            smallest = r;
        }

        if (smallest != i) {
            std::swap(data[i], data[smallest]);
            i = smallest;
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

    // Move last element to root and sift down
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
