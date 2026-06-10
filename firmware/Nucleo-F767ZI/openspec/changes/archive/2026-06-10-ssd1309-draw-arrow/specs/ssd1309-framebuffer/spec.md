## ADDED Requirements

### Requirement: Arrow drawing
The driver SHALL provide
`ssd1309_draw_arrow(p, x0, y0, x1, y1, size, color)` to draw a 1px-wide line
from `(x0, y0)` to `(x1, y1)` (as `ssd1309_draw_line()` does), plus a filled
triangular arrowhead at `(x1, y1)`. The arrowhead's two base corners are
located at approximately ±30° from the shaft's direction and at a distance
of `size + 2` pixels from the tip, giving an arrowhead base width of
approximately `size + 2` pixels. The arrowhead points away from
`(x0, y0)`, in any direction. All drawing is clipped per
`ssd1309_set_pixel()` semantics.

#### Scenario: Draw a horizontal arrow pointing right
- **WHEN** `ssd1309_draw_arrow(p, 10, 32, 30, 32, 6, on)` is called
- **THEN** `ssd1309_get_pixel(p, x, 32)` returns "on" for every `x` from 10
  to 30 inclusive
- **AND** pixels above and below `(30, 32)` near `x` slightly less than 30
  are also "on" (the filled arrowhead)

#### Scenario: Arrow direction follows the line in any orientation
- **WHEN** `ssd1309_draw_arrow(p, x0, y0, x1, y1, size, on)` is called with
  `(x1, y1)` in any direction relative to `(x0, y0)` (e.g. up, down,
  diagonal)
- **THEN** the shaft is drawn from `(x0, y0)` to `(x1, y1)`
- **AND** the filled arrowhead's apex is at `(x1, y1)`, with its base
  corners on `(x0, y0)`'s side, at approximately ±30° from the shaft
  direction

#### Scenario: Zero or negative size draws only the shaft
- **WHEN** `ssd1309_draw_arrow(p, x0, y0, x1, y1, size, on)` is called with
  `size <= 0`
- **THEN** the result is identical to calling
  `ssd1309_draw_line(p, x0, y0, x1, y1, on)`

#### Scenario: Arrow partially outside the panel is clipped
- **WHEN** `ssd1309_draw_arrow(p, x0, y0, x1, y1, size, on)` is called with
  the shaft or arrowhead partially outside `0..width-1` / `0..height-1`
- **THEN** only the in-bounds points are set, per `ssd1309_set_pixel()`'s
  out-of-bounds-is-no-op behavior, and no out-of-bounds memory is written
