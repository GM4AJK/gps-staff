## Why

The driver can draw lines, rectangles, circles, and arrows, but has no
general triangle primitive. A triangle is a useful shape for markers,
indicators, and (combined with the arrowhead fill technique already used by
`ssd1309_draw_arrow()`) is a natural next addition.

## What Changes

- Add `ssd1309_draw_triangle(p, x0, y0, x1, y1, x2, y2, fill, color)`, which
  draws a triangle with vertices `(x0,y0)`, `(x1,y1)`, `(x2,y2)`. When `fill`
  is `false`, only the 1px-wide outline (three edges) is drawn. When `fill`
  is `true`, every pixel within the triangle is set to `color`. Same `bool
  fill` convention as `ssd1309_draw_rect()` / `ssd1309_draw_circle()`.
- Add a demo triangle (outline and/or filled) to `app_init()`.

## Capabilities

### Modified Capabilities
- `ssd1309-framebuffer`: add a triangle-drawing requirement
  (`ssd1309_draw_triangle`).

## Impact

- `Core/Src/ssd1309.c` / `Core/Inc/ssd1309.h`: new function, built on
  `ssd1309_draw_line()`.
- `Core/Src/app.c`: demo update.
