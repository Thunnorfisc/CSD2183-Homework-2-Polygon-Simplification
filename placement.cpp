#include "placement.h"
#include <cmath>
#include <vector>
#include <utility>

CollapseCandidate compute_candidate(Node* A, Node* B, Node* C, Node* D)
{
    CollapseCandidate cand;
    cand.A = A;
    cand.B = B;
    cand.C = C;
    cand.D = D;
    cand.ring_id = A->ring_id;
    cand.vA = A->version;
    cand.vB = B->version;
    cand.vC = C->version;
    cand.vD = D->version;

    // ── Work in local coordinates centered at A for numerical stability ──
    // With coords ~1e6, global cross products give ~1e12 terms that cancel
    // to ~1e3, losing ~9 digits. Local coords keep terms ~1e2, preserving precision.
    double bx = B->x - A->x, by = B->y - A->y;
    double cx = C->x - A->x, cy = C->y - A->y;
    double dx = D->x - A->x, dy = D->y - A->y;

    // Area-preserving line in local coords: a*x + b_coeff*y + c = 0
    // From Equation 1b with A at origin:
    //   a = dy, b = -dx
    //   c = -cy*bx + (by - dy)*cx + cy*dx
    double a_coeff = dy;
    double b_coeff = -dx;
    double c_coeff = -cy * bx + (by - dy) * cx + cy * dx;

    // Degenerate: a=b=0 means A coincides with D
    if (std::abs(a_coeff) < 1e-15 && std::abs(b_coeff) < 1e-15) {
        cand.ex = (B->x + C->x) / 2.0;
        cand.ey = (B->y + C->y) / 2.0;
        cand.displacement = 0.0;
        return cand;
    }

    // Which side of line AD are B and C on?
    // In local coords, AD goes from origin to (dx,dy)
    // side = cross product: dx*py - dy*px
    double sideB = dx * by - dy * bx;
    double sideC = dx * cy - dy * cx;

    // In local coords, sideE_line = c_coeff (since A is at origin)
    double sideE_line = c_coeff;

    // Decide whether to intersect E-line with AB or CD
    bool use_AB;
    bool same_side_of_AD = (sideB * sideC > 0);

    if (same_side_of_AD) {
        // Both on same side: pick the one farther from AD
        // Compare |sideB| vs |sideC| (proportional to distance, same denominator)
        use_AB = (std::abs(sideB) >= std::abs(sideC));
    } else {
        // Opposite sides: pick B's side if it matches E-line's side
        use_AB = (sideB * sideE_line > 0);
    }

    // ── Compute E in local coordinates ──
    double ex_local = 0, ey_local = 0;
    bool ok = false;

    if (use_AB) {
        // Intersect E-line with line from origin (A) to (bx, by)
        // Parametric: P = t*(bx, by), substitute into a*x + b*y + c = 0:
        // t = -c / (a*bx + b*by)
        double denom = a_coeff * bx + b_coeff * by;
        if (std::abs(denom) > 1e-15) {
            double t = -c_coeff / denom;
            ex_local = t * bx;
            ey_local = t * by;
            ok = true;
        }
    } else {
        // Intersect E-line with line from (cx, cy) to (dx, dy)
        // Parametric: P = C + t*(D-C)
        double denom = a_coeff * (dx - cx) + b_coeff * (dy - cy);
        if (std::abs(denom) > 1e-15) {
            double t = -(a_coeff * cx + b_coeff * cy + c_coeff) / denom;
            ex_local = cx + t * (dx - cx);
            ey_local = cy + t * (dy - cy);
            ok = true;
        }
    }

    if (!ok) {
        // Parallel fallback: project midpoint of BC onto E-line
        ex_local = (bx + cx) / 2.0;
        ey_local = (by + cy) / 2.0;
        double val = a_coeff * ex_local + b_coeff * ey_local + c_coeff;
        double norm2 = a_coeff * a_coeff + b_coeff * b_coeff;
        if (norm2 > 1e-15) {
            ex_local -= a_coeff * val / norm2;
            ey_local -= b_coeff * val / norm2;
        }
    }

    // Translate E back to global coordinates
    cand.ex = ex_local + A->x;
    cand.ey = ey_local + A->y;

    // ── Compute areal displacement in local coordinates ──
    // Form 4-vertex polygon (dropping the collinear vertex):
    //   use_AB: E on line AB, so polygon is E,B,C,D
    //   use_CD: E on line CD, so polygon is A(origin),B,C,E

    using Pt = std::pair<double, double>;
    std::vector<Pt> poly;

    if (use_AB) {
        poly = {{ex_local, ey_local}, {bx, by}, {cx, cy}, {dx, dy}};
    } else {
        poly = {{0.0, 0.0}, {bx, by}, {cx, cy}, {ex_local, ey_local}};
    }

    auto shoelace = [](const std::vector<Pt>& pts) -> double {
        double area = 0;
        int n = static_cast<int>(pts.size());
        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            area += pts[i].first * pts[j].second - pts[j].first * pts[i].second;
        }
        return area / 2.0;
    };

    auto seg_intersect = [](double p1x, double p1y, double p2x, double p2y,
                            double p3x, double p3y, double p4x, double p4y,
                            double& ix, double& iy) -> bool {
        double denom = (p2x - p1x) * (p4y - p3y) - (p2y - p1y) * (p4x - p3x);
        if (std::abs(denom) < 1e-15) return false;

        double t = ((p3x - p1x) * (p4y - p3y) - (p3y - p1y) * (p4x - p3x)) / denom;
        double u = ((p3x - p1x) * (p2y - p1y) - (p3y - p1y) * (p2x - p1x)) / denom;

        double eps = 1e-9;
        if (t > eps && t < 1.0 - eps && u > eps && u < 1.0 - eps) {
            ix = p1x + t * (p2x - p1x);
            iy = p1y + t * (p2y - p1y);
            return true;
        }
        return false;
    };

    int n = static_cast<int>(poly.size());
    double ix = 0, iy = 0;
    bool found = false;
    int ei = -1, ej = -1;

    // Check non-adjacent edge pairs for self-intersection
    for (int i = 0; i < n && !found; i++) {
        for (int j = i + 2; j < n; j++) {
            if (i == 0 && j == n - 1) continue; // adjacent in circular sense
            int i1 = (i + 1) % n, j1 = (j + 1) % n;
            if (seg_intersect(poly[i].first, poly[i].second, poly[i1].first, poly[i1].second,
                              poly[j].first, poly[j].second, poly[j1].first, poly[j1].second,
                              ix, iy)) {
                found = true;
                ei = i;
                ej = j;
            }
        }
    }

    if (!found) {
        cand.displacement = std::abs(shoelace(poly));
    } else {
        std::vector<Pt> sub1, sub2;
        sub1.push_back({ix, iy});
        for (int k = (ei + 1) % n; ; k = (k + 1) % n) {
            sub1.push_back(poly[k]);
            if (k == ej) break;
        }
        sub2.push_back({ix, iy});
        for (int k = (ej + 1) % n; ; k = (k + 1) % n) {
            sub2.push_back(poly[k]);
            if (k == ei) break;
        }
        cand.displacement = std::abs(shoelace(sub1)) + std::abs(shoelace(sub2));
    }

    return cand;
}