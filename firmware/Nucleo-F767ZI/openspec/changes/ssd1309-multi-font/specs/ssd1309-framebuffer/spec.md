## MODIFIED Requirements

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
