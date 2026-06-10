## 1. Font descriptor type

- [x] 1.1 Add `ssd1309_font_t` struct to `Core/Inc/ssd1309.h`: `glyphs`,
      `glyph_width`, `glyph_height`, `advance`, `first_char`, `last_char`
- [x] 1.2 Rename the existing glyph table to `font5x7_glyphs` (bytes
      unchanged) and define `const ssd1309_font_t font5x7` wrapping it
      (5x7, advance 6, range `0x20`-`0x7E`)
- [x] 1.3 Declare `font5x7` (and the new fonts from group 2) as `extern
      const ssd1309_font_t` in `Core/Inc/ssd1309.h`

## 2. Medium and large font glyph data

- [x] 2.1 Write a one-off script that scales each `font5x7_glyphs` glyph to
      8x8 (corner-aligned nearest-neighbor) and re-encodes it in the
      column-major / 8-row-chunk layout; generate `font8x8_glyphs[95][8]`
      (replaces an earlier 3x5 "small" font, which was bench-tested and
      found unreadable below 5px wide)
- [x] 2.2 Write a second one-off script that exactly 2x pixel-doubles each
      `font5x7_glyphs` glyph (no resampling) and re-encodes it in the
      column-major / 8-row-chunk layout; generate `font10x14_glyphs[95][20]`
      (replaces an earlier nearest-neighbor-scaled 8x13 font that was
      bench-tested and found illegible)
- [x] 2.3 Add `font8x8_glyphs`/`font10x14_glyphs` tables to `Core/Src/ssd1309.c`
      and define `const ssd1309_font_t font8x8` (8x8, advance 9) and
      `const ssd1309_font_t font10x14` (10x14, advance 12), range `0x20`-`0x7E`
      for both

## 3. Rendering API

- [x] 3.1 Update `ssd1309_draw_char()` signature to
      `void ssd1309_draw_char(ssd1309_t *p, const ssd1309_font_t *font, int16_t x, int16_t y, char c, uint8_t color)`
      and rewrite its glyph loop to iterate `font->glyph_width` columns x
      `(font->glyph_height + 7) / 8` row-chunks x bits, skipping bits where
      `chunk*8+bit >= font->glyph_height`; out-of-range `c` (outside
      `font->first_char`..`font->last_char`) renders blank
- [x] 3.2 Update `ssd1309_draw_string()` signature to
      `void ssd1309_draw_string(ssd1309_t *p, const ssd1309_font_t *font, int16_t x, int16_t y, const char *str, uint8_t color)`,
      advancing `x` by `font->advance` per character
- [x] 3.3 Update both declarations in `Core/Inc/ssd1309.h`

## 4. Sandbox integration

- [x] 4.1 Update the `ssd1309_draw_string()` call in `Core/Src/app.c`
      `app_init()` to pass `&font5x7`

## 5. Bench verification

- [x] 5.1 Build and flash via the external ST-LINK V3, confirm the existing
      status string still renders identically using `font5x7`
- [x] 5.2 Temporarily draw sample strings with `font8x8` and `font10x14` and
      confirm both are legible on the panel; hand-patch any illegible
      glyphs in the generated tables if needed, then revert the temporary
      draw calls
