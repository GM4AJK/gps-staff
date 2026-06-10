## ADDED Requirements

### Requirement: Polygon drawing
The driver SHALL provide a `ssd1309_point_t` struct
(`{ int16_t x; int16_t y; }`) describing a single vertex, and
`ssd1309_draw_polygon(p, points, num_points, fill, color)` to draw a closed
polygon through `points[0]..points[num_points-1]` (`num_points >= 3`). When
`fill` is `false`, only the 1px-wide outline is drawn: an edge between each
consecutive pair of points, plus a closing edge from `points[num_points-1]`
back to `points[0]`. When `fill` is `true`, every pixel within the polygon
(per the even-odd scanline fill rule) is set to `color`. Both cases are
clipped per `ssd1309_set_pixel()` semantics. Polygons with more than
`SSD1309_POLYGON_MAX_POINTS` vertices are not supported.

#### Scenario: Draw a polygon outline
- **WHEN** `ssd1309_draw_polygon(p, points, 4, false, on)` is called with
  `points` describing a square `(10,10)`, `(20,10)`, `(20,20)`, `(10,20)`
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for every point on the
  four edges of the square, including the closing edge from `(10,20)` back
  to `(10,10)`
- **AND** returns "off" for points strictly inside the square (e.g.
  `(15, 15)`)

#### Scenario: Draw a filled polygon
- **WHEN** `ssd1309_draw_polygon(p, points, 4, true, on)` is called with the
  same square as above
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for every point within
  the square, including its edges

#### Scenario: Filled non-convex polygon
- **WHEN** `ssd1309_draw_polygon(p, points, num_points, true, on)` is called
  with a non-convex (e.g. star or "L"-shaped) but non-self-intersecting
  polygon
- **THEN** the even-odd scanline fill sets pixels "on" for every point
  inside the polygon and leaves pixels in concave notches "off"

#### Scenario: Polygon partially outside the panel is clipped
- **WHEN** `ssd1309_draw_polygon(p, points, num_points, fill, on)` is called
  with one or more vertices outside `0..width-1` / `0..height-1`
- **THEN** only the in-bounds points are set, per `ssd1309_set_pixel()`'s
  out-of-bounds-is-no-op behavior, and no out-of-bounds memory is written
