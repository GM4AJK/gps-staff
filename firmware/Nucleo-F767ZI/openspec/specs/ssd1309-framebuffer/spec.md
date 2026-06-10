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
