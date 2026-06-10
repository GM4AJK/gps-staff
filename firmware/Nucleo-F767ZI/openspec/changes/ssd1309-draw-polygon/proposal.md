## Why

The driver can draw lines, rectangles, circles, arrows, and triangles, but
has no general N-sided polygon primitive. A polygon generalizes the
triangle primitive to an arbitrary vertex list, useful for arrows/markers
with more than 3 sides.

## What Changes

- Add a `ssd1309_point_t` struct (`{ int16_t x; int16_t y; }`) describing a
  single vertex.
- Add `ssd1309_draw_polygon(p, points, num_points, fill, color)`, which
  draws a closed polygon through `points[0]..points[num_points-1]`. When
  `fill` is `false`, only the 1px-wide outline (edges between consecutive
  points, plus the closing edge from the last point back to the first) is
  drawn. When `fill` is `true`, every pixel within the polygon is set to
  `color`. Same `bool fill` convention as `ssd1309_draw_rect()` /
  `ssd1309_draw_circle()` / `ssd1309_draw_triangle()`.
- Add a demo polygon (outline and/or filled) to `app_init()`.

## Capabilities

### Modified Capabilities
- `ssd1309-framebuffer`: add a polygon-drawing requirement
  (`ssd1309_draw_polygon`, `ssd1309_point_t`).

## Impact

- `Core/Src/ssd1309.c` / `Core/Inc/ssd1309.h`: new type and function, built
  on `ssd1309_draw_line()`.
- `Core/Src/app.c`: demo update.
