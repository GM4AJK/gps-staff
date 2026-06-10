## Why

The driver now has lines and rectangles. Circles (status indicators,
gauges, dots/markers) are another common UI primitive, following the same
outline/filled pattern established by `ssd1309_draw_rect()`.

## What Changes

- Add `ssd1309_draw_circle(p, x0, y0, r, fill, color)`, drawing a circle
  centered at `(x0, y0)` with radius `r`.
  - `fill == false`: draw only the 1px-wide outline.
  - `fill == true`: fill the entire disc with `color`.
- Handles degenerate cases (`r == 0` draws a single point).

## Capabilities

### New Capabilities
(none)

### Modified Capabilities
- `ssd1309-framebuffer`: add a "Circle drawing" requirement covering
  `ssd1309_draw_circle()`.

## Impact

- `Core/Inc/ssd1309.h`: declare `ssd1309_draw_circle()`.
- `Core/Src/ssd1309.c`: implement `ssd1309_draw_circle()` using the
  midpoint circle algorithm and `ssd1309_set_pixel()`/`ssd1309_draw_line()`.
