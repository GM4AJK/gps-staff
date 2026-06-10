## ADDED Requirements

### Requirement: Rectangle drawing
The driver SHALL provide
`ssd1309_draw_rect(p, x0, y0, x1, y1, fill, color)` to draw a rectangle with
corners `(x0, y0)` and `(x1, y1)` inclusive, in any corner order. When
`fill` is `false`, only the 1px-wide outline (four edges) is drawn. When
`fill` is `true`, every pixel within the rectangle is set to `color`. Both
cases are clipped per `ssd1309_set_pixel()` semantics.

#### Scenario: Draw a rectangle outline
- **WHEN** `ssd1309_draw_rect(p, 10, 10, 20, 15, false, on)` is called
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for every point on the
  four edges of the rectangle from `(10,10)` to `(20,15)`
- **AND** returns "off" for points strictly inside the rectangle (e.g.
  `(15, 12)`)

#### Scenario: Draw a filled rectangle
- **WHEN** `ssd1309_draw_rect(p, 10, 10, 20, 15, true, on)` is called
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for every `x` from 10
  to 20 and every `y` from 10 to 15 inclusive

#### Scenario: Corners given in reverse order
- **WHEN** `ssd1309_draw_rect(p, 20, 15, 10, 10, fill, color)` is called
  (corners reversed relative to the previous scenarios)
- **THEN** the result is identical to calling
  `ssd1309_draw_rect(p, 10, 10, 20, 15, fill, color)`

#### Scenario: Single-point rectangle
- **WHEN** `ssd1309_draw_rect(p, 10, 10, 10, 10, fill, on)` is called (with
  `fill` either `true` or `false`)
- **THEN** `ssd1309_get_pixel(p, 10, 10)` returns "on"

#### Scenario: Rectangle partially outside the panel is clipped
- **WHEN** `ssd1309_draw_rect(p, x0, y0, x1, y1, fill, on)` is called with
  one or more corners outside `0..width-1` / `0..height-1`
- **THEN** only the in-bounds points are set, per `ssd1309_set_pixel()`'s
  out-of-bounds-is-no-op behavior, and no out-of-bounds memory is written
