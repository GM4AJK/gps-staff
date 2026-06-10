## ADDED Requirements

### Requirement: Line drawing
The driver SHALL provide
`ssd1309_draw_line(p, x0, y0, x1, y1, color)` to draw a 1px-wide line
between `(x0, y0)` and `(x1, y1)` inclusive, using Bresenham's algorithm and
`ssd1309_set_pixel()` for each point along the line (so out-of-bounds points
are clipped per `ssd1309_set_pixel()` semantics).

#### Scenario: Draw a horizontal line
- **WHEN** `ssd1309_draw_line(p, 10, 5, 20, 5, on)` is called
- **THEN** `ssd1309_get_pixel(p, x, 5)` returns "on" for every `x` from 10 to
  20 inclusive

#### Scenario: Draw a vertical line
- **WHEN** `ssd1309_draw_line(p, 10, 5, 10, 15, on)` is called
- **THEN** `ssd1309_get_pixel(p, 10, y)` returns "on" for every `y` from 5 to
  15 inclusive

#### Scenario: Draw a diagonal line
- **WHEN** `ssd1309_draw_line(p, 0, 0, 5, 5, on)` is called
- **THEN** `ssd1309_get_pixel(p, n, n)` returns "on" for every `n` from 0 to
  5 inclusive

#### Scenario: Draw a line in either direction
- **WHEN** `ssd1309_draw_line(p, 20, 5, 10, 5, on)` is called (endpoints
  reversed relative to the horizontal-line scenario)
- **THEN** `ssd1309_get_pixel(p, x, 5)` returns "on" for every `x` from 10 to
  20 inclusive

#### Scenario: Draw a single-point line
- **WHEN** `ssd1309_draw_line(p, 10, 10, 10, 10, on)` is called
- **THEN** `ssd1309_get_pixel(p, 10, 10)` returns "on"

#### Scenario: Line endpoints outside the panel are clipped
- **WHEN** `ssd1309_draw_line(p, x0, y0, x1, y1, on)` is called with one or
  both endpoints outside `0..width-1` / `0..height-1`
- **THEN** only the in-bounds points along the line are set, per
  `ssd1309_set_pixel()`'s out-of-bounds-is-no-op behavior, and no
  out-of-bounds memory is written
