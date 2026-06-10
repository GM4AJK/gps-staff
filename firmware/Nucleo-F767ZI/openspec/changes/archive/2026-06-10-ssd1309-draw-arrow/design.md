## Context

The driver provides `ssd1309_draw_line()` (Bresenham), `ssd1309_draw_rect()`,
and `ssd1309_draw_circle()`. An arrow is a line with a "V"-shaped head drawn
at the end point, oriented along the line's direction. The F767 has an FPU,
so `atan2f`/`sinf`/`cosf` are cheap enough for an occasional drawing call.

## Goals / Non-Goals

**Goals:**
- Add `ssd1309_draw_arrow(p, x0, y0, x1, y1, size, color)`: draws a 1px-wide
  line from `(x0, y0)` to `(x1, y1)` and an arrowhead at `(x1, y1)` pointing
  away from `(x0, y0)`, working for a line in any direction.
- `size` is the approximate pixel width of the arrowhead's base.

**Non-Goals:**
- No filled arrowhead, no separate head/shaft colors, no configurable head
  angle.

## Decisions

- **Direction**: compute `angle = atan2f(y1 - y0, x1 - x0)` (screen Y grows
  downward, but `atan2f`/`sinf`/`cosf` are self-consistent so this needs no
  special handling).
- **Head shape**: a filled triangle. Two base corners are computed at
  `angle ± 30°` from the tip `(x1, y1)`, each at distance `size + 2` (a
  couple of pixels longer than `size`, per bench feedback that a length of
  exactly `size` looked too short), giving a base width between the two
  corners of approximately `2 * (size + 2) * sin(30°) = size + 2`.
- **Base corners**:
  `(x1 - (size+2)*cos(angle - 30°), y1 - (size+2)*sin(angle - 30°))` and
  `(x1 - (size+2)*cos(angle + 30°), y1 - (size+2)*sin(angle + 30°))`.
- **Fill technique**: draw a fan of `2*size + 4` lines via
  `ssd1309_draw_line()`, each from the tip `(x1, y1)` to a point linearly
  interpolated between the two base corners. This is dense enough to leave
  no gaps for the small sizes used here, reuses `ssd1309_draw_line()` for
  clipping/dirty-flag consistency, and avoids needing a general polygon-fill
  primitive.
- **Degenerate cases**: `size <= 0` draws only the shaft line (no
  arrowhead). A zero-length shaft (`x0==x1 && y0==y1`) still draws an
  arrowhead using `atan2f(0, 0) == 0` (pointing in the +x direction) — an
  acceptable, harmless default for a degenerate input.

## Risks / Trade-offs

- [Floating-point trig per call] → negligible cost for an occasional
  drawing call on an FPU-equipped MCU.
- [Rounding of wing endpoints to `int16_t` may make the base width differ
  from `size` by a pixel or two] → acceptable, `size` is documented as
  "approximate".
