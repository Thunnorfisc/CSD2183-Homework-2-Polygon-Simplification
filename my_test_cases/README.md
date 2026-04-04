# My Test Cases for Polygon Simplification

Each test case targets a specific robustness aspect of the simplification algorithm.

## Test Cases

| Input file | Target | Description |
|---|---|---|
| `input_triangle_minimal.csv` | 3 | Edge case: already at minimum vertices, nothing to simplify |
| `input_star_with_center_hole.csv` | 8 | Concave star with sharp spikes and a hole near the center |
| `input_thin_zigzag_strip.csv` | 10 | Very thin strip with zigzag edges - tests near-degenerate geometry |
| `input_arrow_shape.csv` | 7 | Non-convex arrow with notch - tests concavity preservation |
| `input_nested_squares.csv` | 12 | 3 concentric square holes - tests multi-ring topology with minimal vertices per ring |
| `input_donut_ring.csv` | 12 | Circular donut with tight inner/outer gap - tests hole proximity |
| `input_circle_approx_64.csv` | 16 | 64-vertex circle reduced heavily - tests uniform curvature simplification |
| `input_spiral_polygon.csv` | 15 | Self-approaching spiral corridor - tests narrow passages and near self-intersection |
| `input_many_small_holes.csv` | 50 | Grid of 25 square holes - stress test for many rings |
| `input_large_jagged_coastline.csv` | 20 | 96-vertex jagged coastline - tests noisy high-frequency detail removal |
