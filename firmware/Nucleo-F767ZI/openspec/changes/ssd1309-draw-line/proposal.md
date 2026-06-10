## Why

The SSD1309 driver currently only supports per-pixel drawing
(`ssd1309_set_pixel`/`ssd1309_get_pixel`) and text. Future sandbox screens
(graphs, separators, simple gauges/UI chrome) need a basic line-drawing
primitive rather than every caller re-implementing Bresenham.

## What Changes

- Add `ssd1309_draw_line(p, x0, y0, x1, y1, color)`, drawing a 1px-wide line
  between two arbitrary points using Bresenham's algorithm, built on
  `ssd1309_set_pixel()` (so out-of-bounds endpoints/segments are clipped for
  free).
- Handles all line orientations (horizontal, vertical, diagonal in any
  direction) and degenerate cases (single point, zero-length line).

## Capabilities

### New Capabilities
(none)

### Modified Capabilities
- `ssd1309-framebuffer`: add a "Line drawing" requirement covering
  `ssd1309_draw_line()`.

## Impact

- `Core/Inc/ssd1309.h`: declare `ssd1309_draw_line()`.
- `Core/Src/ssd1309.c`: implement `ssd1309_draw_line()` using Bresenham's
  algorithm and `ssd1309_set_pixel()`.
