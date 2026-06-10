## Context

The driver provides `ssd1309_draw_line()`, `ssd1309_draw_rect()`,
`ssd1309_draw_circle()`, and `ssd1309_draw_triangle()`, the last using a
sort-by-y scanline fill specialized for 3 vertices. A general polygon needs
a vertex-count-agnostic scanline fill.

## Goals / Non-Goals

**Goals:**
- Add a `ssd1309_point_t { int16_t x; int16_t y; }` struct.
- Add `ssd1309_draw_polygon(p, points, num_points, fill, color)` covering
  outline and filled N-gons (N >= 3) with one function and a `bool fill`
  flag, matching the rect/circle/triangle convention.
- Support non-convex (but non-self-intersecting) polygons via an even-odd
  scanline fill.

**Non-Goals:**
- No arbitrary self-intersecting polygon correctness guarantees beyond
  whatever the even-odd rule naturally produces.
- No dynamic allocation; a fixed maximum vertex count is acceptable.

## Decisions

- **Outline case**: draw an edge from `points[i]` to
  `points[(i+1) % num_points]` for each `i` via `ssd1309_draw_line()` (this
  also closes the polygon from the last point back to the first).
- **Filled case**: classic even-odd scanline fill (the
  "public-domain polygon fill" algorithm):
  1. For each scanline `y` from the polygon's min `y` to max `y`:
     - For each edge `(p1, p2)` where exactly one of `p1.y <= y` /
       `p2.y <= y` holds (i.e. the edge crosses the scanline, with
       horizontal edges skipped and each vertex counted once), compute the
       x-intersection via linear interpolation and add it to a list.
     - Sort the x-intersections ascending.
     - Draw a horizontal span via `ssd1309_draw_line()` for each consecutive
       pair of intersections (`[0]-[1]`, `[2]-[3]`, ...).
- **Fixed intersection buffer**: a stack array of
  `SSD1309_POLYGON_MAX_POINTS` (32) `int16_t` x-intersections per scanline —
  at most one intersection per edge, so this supports polygons with up to 32
  vertices. `num_points > 32` is not expected for this sandbox's use cases.
- **`ssd1309_point_t`**: a small `{ int16_t x; int16_t y; }` struct, added to
  `ssd1309.h`, used for the `points` array parameter.

## Risks / Trade-offs

- [Fixed 32-point intersection buffer caps polygon complexity] →
  acceptable for the sandbox's marker/indicator use cases; documented in the
  header comment.
- [Even-odd fill on self-intersecting polygons may look unexpected] →
  acceptable; not a goal to handle that case specially.
