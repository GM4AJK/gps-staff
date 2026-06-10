## Context

The driver provides `ssd1309_set_pixel()`, `ssd1309_draw_line()`, and
`ssd1309_draw_rect()` (with an outline/filled `bool fill` flag). Circles are
the next common primitive (status dots, gauges, markers), and should follow
the same `fill` flag convention for API consistency.

## Goals / Non-Goals

**Goals:**
- Add `ssd1309_draw_circle(p, x0, y0, r, fill, color)` covering both
  outline and filled circles with one function and a `bool fill` flag,
  matching `ssd1309_draw_rect()`'s convention.
- `r == 0` draws a single point regardless of `fill`.
- Reuse existing primitives for clipping/dirty-flag behavior.

**Non-Goals:**
- No ellipses, arcs, or separate fill/outline colors in one call.

## Decisions

- **Algorithm**: integer midpoint circle algorithm (the common
  `f`/`ddF_x`/`ddF_y` formulation, as used by e.g. Adafruit GFX) — standard,
  integer-only, symmetric across all 8 octants. An earlier
  `err <= 0` / `err > 0` formulation was tried first but produced a visibly
  lumpy/asymmetric circle and was replaced with this variant.
- **Outline case**: seed the 4 cardinal points (`(x0, y0±r)`,
  `(x0±r, y0)`), then for each computed `(x, y)` step, plot all 8 symmetric
  points via `ssd1309_set_pixel()`.
- **Filled case**: seed a vertical center line (`(x0, y0-r)`..`(x0, y0+r)`)
  and a horizontal diameter line (`(x0-r, y0)`..`(x0+r, y0)`) — the loop's
  `(x, y)` steps never produce a span at row `y0` itself, so the diameter
  row must be seeded explicitly — then for each computed `(x, y)` step, draw
  horizontal spans via
  `ssd1309_draw_line()` connecting the symmetric points across each of the 4
  horizontal-span pairs (top/bottom halves), avoiding redundant re-drawing of
  the same span on every iteration by drawing per row as the algorithm
  progresses.

## Risks / Trade-offs

- [Filled case may draw the same horizontal span more than once for
  adjacent midpoint-algorithm steps at shallow angles] → harmless,
  `ssd1309_set_pixel()`/`ssd1309_draw_line()` are idempotent and only mark
  dirty on actual change.
