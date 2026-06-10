## Why

The framebuffer/text change (`ssd1309-framebuffer`) added a single built-in
5x7 font with a hard-coded 6px advance baked into `ssd1309_draw_char()` /
`ssd1309_draw_string()`. Future screens need a range of text sizes (e.g. a
small status line vs. a large heading), which the current API cannot express
without duplicating the draw functions per font.

## What Changes

- Introduce a `ssd1309_font_t` struct describing a font: glyph bitmap table,
  glyph pixel width/height, per-glyph advance (cell width), and the covered
  character range (`first_char`/`last_char`).
- Wrap the existing 5x7 glyph table in a `const ssd1309_font_t font5x7`
  definition (no bitmap changes). On a 128x64 panel, fonts smaller than 5x7
  are not legibly readable, so `font5x7` serves as the smallest of the
  three built-in fonts rather than gaining a smaller sibling.
- Add two new built-in fonts: a medium font (8x8, scaled up from the 5x7
  glyphs) and a large font (10x14, an exact 2x pixel-doubling of the 5x7
  glyphs), each as their own `const ssd1309_font_t`.
- **BREAKING**: `ssd1309_draw_char()` and `ssd1309_draw_string()` gain a
  `const ssd1309_font_t *font` parameter, inserted after `p` and before the
  position arguments. All existing call sites must be updated to pass
  `&font5x7` (or another font) explicitly.
- Update the sandbox `app_init()` status-string call to pass `&font5x7`.

## Capabilities

### New Capabilities
(none)

### Modified Capabilities
- `ssd1309-framebuffer`: the "Monospace text rendering" requirement changes
  from a single built-in font to a font-parameterized API backed by a
  `ssd1309_font_t` descriptor, with at least three built-in fonts (5x7, 8x8,
  10x14).

## Impact

- `Core/Inc/ssd1309.h`: new `ssd1309_font_t` struct, updated
  `ssd1309_draw_char`/`ssd1309_draw_string` signatures, font declarations
  (`font5x7`, `font8x8`, `font10x14`).
- `Core/Src/ssd1309.c`: refactor `font5x7` into a `ssd1309_font_t`, add new
  glyph tables for the medium and large fonts, update `draw_char`/
  `draw_string` to use the font descriptor for glyph lookup and advance.
- `Core/Src/app.c`: update the existing `ssd1309_draw_string()` call to pass
  `&font5x7`.
