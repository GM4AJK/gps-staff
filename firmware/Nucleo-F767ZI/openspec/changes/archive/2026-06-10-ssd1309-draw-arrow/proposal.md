## Why

The driver can draw lines, rectangles, and circles, but has no way to
indicate direction (e.g. heading/bearing markers, pointers on a gauge).
An arrowhead drawn at the end of a line is a common way to add this.

## What Changes

- Add `ssd1309_draw_arrow(p, x0, y0, x1, y1, size, color)`, which draws a
  1px-wide line from `(x0, y0)` to `(x1, y1)` (as `ssd1309_draw_line()`
  does) and an arrowhead at `(x1, y1)` pointing in the direction from
  `(x0, y0)` to `(x1, y1)`. `size` is the approximate pixel width of the
  arrowhead's base, and the arrowhead can point in any direction.
- Add a demo arrow to `app_init()`.

## Capabilities

### Modified Capabilities
- `ssd1309-framebuffer`: add an arrow-drawing requirement
  (`ssd1309_draw_arrow`).

## Impact

- `Core/Src/ssd1309.c` / `Core/Inc/ssd1309.h`: new function, built on
  `ssd1309_draw_line()`.
- `Core/Src/app.c`: demo update.
