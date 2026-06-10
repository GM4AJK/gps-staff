## ADDED Requirements

### Requirement: Circle drawing
The driver SHALL provide
`ssd1309_draw_circle(p, x0, y0, r, fill, color)` to draw a circle centered at
`(x0, y0)` with radius `r`, using the midpoint circle algorithm. When `fill`
is `false`, only the 1px-wide outline is drawn. When `fill` is `true`, every
pixel within the disc is set to `color`. Both cases are clipped per
`ssd1309_set_pixel()` semantics.

#### Scenario: Draw a circle outline
- **WHEN** `ssd1309_draw_circle(p, 32, 32, 10, false, on)` is called
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for points on the
  circle's circumference (e.g. `(42, 32)`, `(22, 32)`, `(32, 42)`,
  `(32, 22)`)
- **AND** returns "off" for the center point `(32, 32)`

#### Scenario: Draw a filled circle
- **WHEN** `ssd1309_draw_circle(p, 32, 32, 10, true, on)` is called
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for the center point
  `(32, 32)` and for every point within the disc of radius 10 centered on
  `(32, 32)`

#### Scenario: Zero-radius circle draws a single point
- **WHEN** `ssd1309_draw_circle(p, 10, 10, 0, fill, on)` is called (with
  `fill` either `true` or `false`)
- **THEN** `ssd1309_get_pixel(p, 10, 10)` returns "on"
- **AND** no other pixels are set

#### Scenario: Circle partially outside the panel is clipped
- **WHEN** `ssd1309_draw_circle(p, x0, y0, r, fill, on)` is called with the
  circle's bounding box partially outside `0..width-1` / `0..height-1`
- **THEN** only the in-bounds points are set, per `ssd1309_set_pixel()`'s
  out-of-bounds-is-no-op behavior, and no out-of-bounds memory is written
