## Context

The driver provides `ssd1309_set_pixel()` (bounds-checked, dirty-tracking)
and `ssd1309_draw_line()` (Bresenham, built on `set_pixel`). Rectangles are
the next common UI primitive (borders, boxes, filled bars/gauges).

## Goals / Non-Goals

**Goals:**
- Add `ssd1309_draw_rect(p, x0, y0, x1, y1, fill, color)` covering both
  outline and filled rectangles with one function and a `bool fill` flag.
- Accept corners in any order (normalize internally).
- Reuse existing primitives (`ssd1309_draw_line`, `ssd1309_set_pixel`) for
  consistency and to inherit clipping/dirty-flag behavior for free.

**Non-Goals:**
- No rounded corners, border thickness, or separate fill/outline colors in
  one call.

## Decisions

- **Single function with a `bool fill` flag**, as requested, rather than
  two separate functions (`draw_rect_outline`/`draw_rect_filled`) — keeps
  the API surface small and matches the line primitive's simple signature
  style.
- **Outline case**: normalize `(x0,y0)`-`(x1,y1)` to `(min_x,min_y)`-`(max_x,max_y)`,
  then draw the four edges with `ssd1309_draw_line()` (top, bottom, left,
  right). For a single-row or single-column rectangle this naturally
  degenerates to one or two lines without special-casing.
- **Filled case**: normalize corners, then iterate rows and call
  `ssd1309_set_pixel()` for each pixel in the rectangle (simple nested loop
  — rectangles here are small, max 128x64).

## Risks / Trade-offs

- [Outline case draws corner pixels twice (once per adjacent edge)] →
  harmless, `ssd1309_set_pixel()` is idempotent and only marks dirty on
  actual change.
