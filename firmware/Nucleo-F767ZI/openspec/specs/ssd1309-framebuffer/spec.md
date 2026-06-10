## Purpose

SSD1309 OLED panel framebuffer, drawing primitives, and text rendering for
the Nucleo-F767ZI sandbox: a 1bpp RAM framebuffer mirroring the panel's
GDDRAM layout, pixel-level set/get and clear/fill operations, a dirty-flag
flush to push the framebuffer to the panel over I2C, and multi-font
monospace text rendering.

## Requirements

### Requirement: Framebuffer storage
The driver SHALL provide a 1bpp RAM framebuffer within `ssd1309_t`, organized
in the same page-major layout as SSD1309 GDDRAM (one byte per 8 vertical
pixels of a single column, bit 0 = top row of the page), sized to support
panels up to 128x64.

#### Scenario: Buffer sized for the configured panel
- **WHEN** a handle is initialized via `ssd1309_init()` with `height` and
  `width`
- **THEN** the driver uses the first `width * (height / 8)` bytes of the
  framebuffer for all drawing/flush operations on that handle

### Requirement: Clear and fill
The driver SHALL provide `ssd1309_clear()` to set every pixel in the
framebuffer to off, and `ssd1309_fill()` to set every pixel to a given
on/off color. Both operations SHALL mark the framebuffer dirty
unconditionally.

#### Scenario: Clear sets all pixels off
- **WHEN** `ssd1309_clear(p)` is called
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "off" for every `x`, `y`
  within the panel bounds
- **AND** the framebuffer is marked dirty

#### Scenario: Fill sets all pixels to a color
- **WHEN** `ssd1309_fill(p, color)` is called with color "on"
- **THEN** `ssd1309_get_pixel(p, x, y)` returns "on" for every `x`, `y`
  within the panel bounds
- **AND** the framebuffer is marked dirty

### Requirement: Pixel set/get
The driver SHALL provide `ssd1309_set_pixel(p, x, y, color)` to set or clear
a single framebuffer pixel, and `ssd1309_get_pixel(p, x, y)` to read it back.
Coordinates outside the panel's `width`/`height` SHALL be silently ignored
(for `set_pixel`) or treated as off (for `get_pixel`), without modifying
out-of-bounds memory or returning garbage. `ssd1309_set_pixel()` SHALL mark
the framebuffer dirty only when it actually changes a pixel's value.

#### Scenario: Set and read back an in-bounds pixel
- **WHEN** `ssd1309_set_pixel(p, x, y, on)` is called with `0 <= x < width`
  and `0 <= y < height`
- **THEN** a subsequent `ssd1309_get_pixel(p, x, y)` returns "on"

#### Scenario: Setting a pixel to a new value marks the framebuffer dirty
- **WHEN** `ssd1309_set_pixel(p, x, y, color)` is called with an in-bounds
  `x`, `y` whose current value differs from `color`
- **THEN** the pixel's value is updated
- **AND** the framebuffer is marked dirty

#### Scenario: Setting a pixel to its current value does not mark dirty
- **WHEN** `ssd1309_set_pixel(p, x, y, color)` is called with an in-bounds
  `x`, `y` whose current value already equals `color`, and the framebuffer
  is not already dirty
- **THEN** the framebuffer remains not dirty

#### Scenario: Out-of-bounds set is a no-op
- **WHEN** `ssd1309_set_pixel(p, x, y, on)` is called with `x >= width` or
  `y >= height`
- **THEN** the framebuffer contents are unchanged
- **AND** the dirty state is unchanged

#### Scenario: Out-of-bounds get returns off
- **WHEN** `ssd1309_get_pixel(p, x, y)` is called with `x >= width` or
  `y >= height`
- **THEN** the function returns "off" without reading out-of-bounds memory

### Requirement: Flush framebuffer to panel
The driver SHALL provide `ssd1309_flush(p)`, which writes the entire
framebuffer to GDDRAM via page-addressing mode (one I2C data write per page,
preceded by a column-address-reset command write per page), using the
handle's stored I2C peripheral and 7-bit address, but only when the
framebuffer is marked dirty. `ssd1309_flush()` SHALL be callable any number
of times after `ssd1309_bringup()` has completed successfully.

#### Scenario: Flush is a no-op when not dirty
- **WHEN** `ssd1309_flush(p)` is called and the framebuffer is not marked
  dirty
- **THEN** no I2C transfers are performed
- **AND** the function returns `HAL_OK`

#### Scenario: Successful flush returns HAL_OK and clears dirty
- **WHEN** `ssd1309_flush(p)` is called while the framebuffer is marked
  dirty, after `ssd1309_bringup(p)` has returned `HAL_OK`, and the panel
  acknowledges all I2C transfers
- **THEN** for each page (0 to `height/8 - 1`) the driver resets the column
  address to 0 and writes that page's `width` framebuffer bytes to GDDRAM
  with the I2C data control byte (`0x40`)
- **AND** returns `HAL_OK`
- **AND** the framebuffer is no longer marked dirty

#### Scenario: I2C failure during flush is propagated and dirty remains set
- **WHEN** any I2C transfer during `ssd1309_flush(p)` fails (NACK, bus
  error, or timeout) while the framebuffer is marked dirty
- **THEN** `ssd1309_flush(p)` returns the corresponding non-`HAL_OK`
  `HAL_StatusTypeDef` from the failing `HAL_I2C_*` call without sending
  further pages
- **AND** the framebuffer remains marked dirty

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

### Requirement: Monospace text rendering
The driver SHALL provide a `ssd1309_font_t` descriptor type describing a
fixed-width font: a flat glyph bitmap table, glyph pixel width, glyph pixel
height, per-character advance (in pixels), and the inclusive printable
character range it covers (`first_char`..`last_char`). Glyph bitmaps SHALL
be stored column-major, with `(glyph_height + 7) / 8` bytes per column, bit 0
of each byte being the top row of its 8-row chunk.

The driver SHALL provide at least three built-in fonts, each covering
printable ASCII `0x20`-`0x7E`:
- `font5x7`: 5x7 glyphs, 6px advance (the glyph bitmaps are unchanged from
  the original single-font implementation).
- `font8x8`: 8x8 glyphs, 9px advance.
- `font10x14`: 10x14 glyphs, 12px advance.

The driver SHALL provide
`ssd1309_draw_char(p, font, x, y, c, color)` to draw a single glyph from the
given font into the framebuffer with its top-left corner at `(x, y)`, and
`ssd1309_draw_string(p, font, x, y, str, color)` to draw a NUL-terminated
string using the given font, advancing the x position by `font->advance`
pixels per character with no wrapping.

#### Scenario: Draw a printable character in a given font
- **WHEN** `ssd1309_draw_char(p, font, x, y, c, on)` is called with `c`
  within `font->first_char`..`font->last_char`
- **THEN** the corresponding glyph bitmap from `font` is written into the
  framebuffer at `(x, y)` using `ssd1309_set_pixel()` semantics (including
  clipping at panel edges), covering `font->glyph_width` x
  `font->glyph_height` pixels

#### Scenario: Draw a string advances by the font's advance per character
- **WHEN** `ssd1309_draw_string(p, font, x, y, "AB", on)` is called
- **THEN** the glyph for `'A'` is drawn at `(x, y)` and the glyph for `'B'`
  is drawn at `(x + font->advance, y)`

#### Scenario: Out-of-range characters render blank
- **WHEN** `ssd1309_draw_char(p, font, x, y, c, color)` is called with `c`
  outside `font->first_char`..`font->last_char`
- **THEN** the framebuffer region for that character cell is left unchanged
  (no glyph pixels are set)

#### Scenario: The 5x7 font matches the original single-font glyphs
- **WHEN** `ssd1309_draw_char(p, &font5x7, x, y, c, on)` is called with `c`
  in `0x20`-`0x7E`
- **THEN** the glyph drawn is pixel-identical to the original single-font
  implementation's 5x7 bitmap for that character, drawn within a 6x8 cell
