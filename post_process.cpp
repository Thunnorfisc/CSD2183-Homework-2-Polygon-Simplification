#include "post_process.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>

using Pt = std::pair<double, double>;

static std::vector<Pt> ring_to_vec(const Ring& ring)
{
    std::vector<Pt> v;
    Node* cur = ring.vertices.head;
    do {
        v.push_back({cur->x, cur->y});
        cur = cur->next;
    } while (cur != ring.vertices.head);
    return v;
}

static bool causes_self_intersection(Node* V, double nx, double ny)
{
    Node* A = V->prev;
    Node* D = V->next;
    Node* cur = D->next;
    if (cur == A) return false;
    Node* stop = A;

    auto segs_cross = [](double p1x, double p1y, double p2x, double p2y,
                         double p3x, double p3y, double p4x, double p4y) -> bool {
        auto cross_val = [](double ax, double ay, double bx, double by, double cx, double cy) {
            return (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
        };
        double d1 = cross_val(p3x, p3y, p4x, p4y, p1x, p1y);
        double d2 = cross_val(p3x, p3y, p4x, p4y, p2x, p2y);
        double d3 = cross_val(p1x, p1y, p2x, p2y, p3x, p3y);
        double d4 = cross_val(p1x, p1y, p2x, p2y, p4x, p4y);
        return ((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
               ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0));
    };

    do {
        Node* nxt = cur->next;
        if (cur != A && nxt != A && cur != D && nxt != D) {
            if (segs_cross(A->x, A->y, nx, ny, cur->x, cur->y, nxt->x, nxt->y))
                return true;
            if (segs_cross(nx, ny, D->x, D->y, cur->x, cur->y, nxt->x, nxt->y))
                return true;
        }
        cur = nxt;
    } while (cur != stop);

    return false;
}

static int remove_collinear(Ring& ring)
{
    if (ring.vertices.size <= 3) return 0;
    int removed = 0;
    Node* cur = ring.vertices.head;
    Node* start = cur;
    bool first = true;

    while (first || cur != start) {
        first = false;
        if (ring.vertices.size <= 3) break;
        Node* A = cur->prev;
        Node* B = cur;
        Node* C = cur->next;
        double cv = (B->x - A->x) * (C->y - A->y) - (B->y - A->y) * (C->x - A->x);
        if (std::abs(cv) < 1e-9) {
            Node* next = C;
            ring.vertices.remove(B);
            removed++;
            if (start == B) start = next;
            cur = next;
        } else {
            cur = cur->next;
        }
    }
    return removed;
}

int post_process(std::vector<Ring>& rings,
                 const std::vector<std::vector<Pt>>& original_vertices,
                 int max_iterations)
{
    int total_relocated = 0;

    for (size_t ri = 0; ri < rings.size(); ri++) {
        Ring& ring = rings[ri];
        const std::vector<Pt>& orig = original_vertices[ri];

        if (ring.vertices.size <= 3 || orig.size() < 3) continue;

        int col_removed = remove_collinear(ring);
        if (col_removed > 0) {
            std::cerr << "  Ring " << ring.ring_id << ": removed "
                      << col_removed << " collinear vertices" << std::endl;
        }

        double best_sym_diff = compute_symmetric_difference(orig, ring_to_vec(ring));
        std::cerr << "  Ring " << ring.ring_id << ": initial sym diff = " << best_sym_diff << std::endl;

        for (int iter = 0; iter < max_iterations; iter++) {
            int relocated = 0;

            Node* cur = ring.vertices.head;
            do {
                Node* A = cur->prev;
                Node* V = cur;
                Node* D = cur->next;

                double dx = D->x - A->x, dy = D->y - A->y;
                double len = std::sqrt(dx * dx + dy * dy);
                if (len < 1e-12) { cur = cur->next; continue; }
                dx /= len;
                dy /= len;

                double step = len * 0.25;
                double orig_x = V->x, orig_y = V->y;
                double best_x = orig_x, best_y = orig_y;
                double best_local = best_sym_diff;

                // Fewer trials for speed: 5 positions instead of 11
                double trials[] = {-1.0, -0.3, 0.3, 1.0, -0.1, 0.1};

                for (double t : trials) {
                    double nx = orig_x + t * step * dx;
                    double ny = orig_y + t * step * dy;

                    if (causes_self_intersection(V, nx, ny)) continue;

                    V->x = nx;
                    V->y = ny;
                    double trial_diff = compute_symmetric_difference(orig, ring_to_vec(ring));
                    if (trial_diff < best_local - 1e-6) {
                        best_local = trial_diff;
                        best_x = nx;
                        best_y = ny;
                    }
                    V->x = orig_x;
                    V->y = orig_y;
                }

                if (best_x != orig_x || best_y != orig_y) {
                    V->x = best_x;
                    V->y = best_y;
                    best_sym_diff = best_local;
                    relocated++;
                }

                cur = cur->next;
            } while (cur != ring.vertices.head);

            total_relocated += relocated;
            std::cerr << "  Ring " << ring.ring_id << " iter " << iter
                      << ": relocated " << relocated
                      << " vertices, sym diff = " << best_sym_diff << std::endl;

            if (relocated == 0) break;
        }
    }

    return total_relocated;
}