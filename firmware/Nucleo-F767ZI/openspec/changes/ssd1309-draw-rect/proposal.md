## Why

The driver now has line-drawing (`ssd1309_draw_line`), but rectangles
(panel borders, boxes, filled gauges/bars) still have to be hand-built from
four lines. A single rectangle primitive covering both outline and filled
cases is a natural next building block for UI chrome.

## What Changes

- Add `ssd1309_draw_rect(p, x0, y0, x1, y1, fill, color)`, drawing a
  rectangle with corners `(x0, y0)` and `(x1, y1)` inclusive.
  - `fill == false`: draw only the 1px-wide outline (4 edges).
  - `fill == true`: fill the entire rectangle area with `color`.
- Handles corners given in any order (e.g. `x1 < x0` and/or `y1 < y0`) and
  degenerate cases (single row/column, single point).

## Capabilities

### New Capabilities
(none)

### Modified Capabilities
- `ssd1309-framebuffer`: add a "Rectangle drawing" requirement covering
  `ssd1309_draw_rect()`.

## Impact

- `Core/Inc/ssd1309.h`: declare `ssd1309_draw_rect()`.
- `Core/Src/ssd1309.c`: implement `ssd1309_draw_rect()`, reusing
  `ssd1309_draw_line()` for the outline case and `ssd1309_set_pixel()` for
  the filled case.
