## ADDED Requirements

### Requirement: Triangle drawing
The driver SHALL provide
`ssd1309_draw_triangle(p, x0, y0, x1, y1, x2, y2, fill, color)` to draw a
triangle with vertices `(x0,y0)`, `(x1,y1)`, `(x2,y2)`. When `fill` is
`false`, only the 1px-wide outline (three edges) is drawn. When `fill` is
`true`, every pixel within the triangle is set to `color`. Both cases are
clipped per `ssd1309_set_pixel()` semantics.

#### Scenario: Draw a triangle outline
- **WHEN** `ssd1309_draw_triangle(p, 10, 10, 20, 10, 10, 20, false, on)` is
  called
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for every point on the
  three edges connecting `(10,10)`, `(20,10)`, and `(10,20)`
- **AND** returns "off" for points strictly inside the triangle (e.g.
  `(13, 13)`)

#### Scenario: Draw a filled triangle
- **WHEN** `ssd1309_draw_triangle(p, 10, 10, 20, 10, 10, 20, true, on)` is
  called
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for every point within
  the triangle bounded by `(10,10)`, `(20,10)`, and `(10,20)`, including its
  edges

#### Scenario: Vertex order does not matter
- **WHEN** `ssd1309_draw_triangle(p, x2, y2, x0, y0, x1, y1, fill, color)` is
  called with the same three vertices in a different order
- **THEN** the result is identical to calling
  `ssd1309_draw_triangle(p, x0, y0, x1, y1, x2, y2, fill, color)`

#### Scenario: Degenerate triangle with collinear or repeated vertices
- **WHEN** `ssd1309_draw_triangle(p, x0, y0, x1, y1, x2, y2, fill, on)` is
  called with all three vertices collinear (including the case where two or
  three vertices are identical)
- **THEN** the pixels along the line(s) connecting the distinct vertices are
  set to "on"
- **AND** no out-of-bounds memory is written

#### Scenario: Triangle partially outside the panel is clipped
- **WHEN** `ssd1309_draw_triangle(p, x0, y0, x1, y1, x2, y2, fill, on)` is
  called with one or more vertices outside `0..width-1` / `0..height-1`
- **THEN** only the in-bounds points are set, per `ssd1309_set_pixel()`'s
  out-of-bounds-is-no-op behavior, and no out-of-bounds memory is written
