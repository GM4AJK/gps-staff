## Context

The driver provides `ssd1309_draw_line()`, `ssd1309_draw_rect()`, and
`ssd1309_draw_circle()`, all with a `bool fill` flag for outline-vs-filled.
A general triangle (not just the small near-isoceles shape used by
`ssd1309_draw_arrow()`'s fan-fill) needs a fill technique that works for
arbitrary, possibly elongated, triangles.

## Goals / Non-Goals

**Goals:**
- Add `ssd1309_draw_triangle(p, x0, y0, x1, y1, x2, y2, fill, color)`
  covering both outline and filled triangles with one function and a `bool
  fill` flag, matching the existing rect/circle convention.
- Handle degenerate triangles (collinear or repeated points) without
  special-casing beyond what the chosen algorithm naturally handles.

**Non-Goals:**
- No anti-aliasing, no separate fill/outline colors in one call.

## Decisions

- **Outline case**: draw the three edges via
  `ssd1309_draw_line()`: `(x0,y0)-(x1,y1)`, `(x1,y1)-(x2,y2)`,
  `(x2,y2)-(x0,y0)`.
- **Filled case**: standard scanline fill (the well-known
  "sort by y, split into flat-bottom/flat-top halves" technique used by e.g.
  Adafruit GFX's `fillTriangle`):
  1. Sort the three vertices by `y` ascending: `(x0,y0)`, `(x1,y1)`,
     `(x2,y2)` become `a`, `b`, `c` with `a.y <= b.y <= c.y`.
  2. For each scanline `y` from `a.y` to `c.y`, compute the x-intersections
     of that scanline with edge `a-c` and with edge `a-b` (for `y <= b.y`)
     or edge `b-c` (for `y > b.y`), using integer-friendly linear
     interpolation.
  3. Draw a horizontal span via `ssd1309_draw_line()` between the two
     x-intersections for each scanline.
  - This naturally handles degenerate triangles: a zero-height triangle
    (all `y` equal) draws nothing extra beyond the outline-equivalent span;
    collinear points produce zero-width spans (single pixels).

## Risks / Trade-offs

- [Scanline fill is more code than the arrow's fan-fill] → justified because
  a general triangle can be much more elongated than an arrowhead, where
  fan-fill would leave visible gaps.
- [Integer division/interpolation per scanline] → negligible cost for an
  occasional drawing call; reuses `ssd1309_draw_line()` for clipping/dirty
  consistency.
