## Why

The SSD1309 driver can now power on the panel and light every pixel
(`ssd1309_bringup()`, see `ssd1309-display-bringup`), but there is no way to
draw anything meaningful — every "display" is the bring-up smoke test. To
make the panel useful on the bench (status text, sensor readouts, simple
graphics for future modules), the driver needs a RAM framebuffer, basic
drawing primitives, a way to push the buffer to the panel, and minimal text
rendering for debug output.

## What Changes

- Add a 1bpp RAM framebuffer sized to the panel (`128x64` by default,
  1024 bytes, page-major layout matching SSD1309 GDDRAM organization).
- Add framebuffer primitives: `ssd1309_clear()`, `ssd1309_set_pixel()`,
  `ssd1309_get_pixel()`, `ssd1309_fill()`.
- Add `ssd1309_flush()`: writes the entire framebuffer to GDDRAM via
  page-addressing mode, blocking I2C, reusing the same write pattern as the
  existing test-pattern code in `ssd1309_bringup()`. Full-buffer flush only
  (no dirty-region tracking).
- Add a built-in fixed-width monospace font (5x7 glyphs in an 8px-tall
  cell) covering printable ASCII, plus `ssd1309_draw_char()` and
  `ssd1309_draw_string()` that write glyphs into the framebuffer at a given
  pixel position.
- Update the sandbox `app.c` bring-up: after `ssd1309_bringup()`, clear the
  framebuffer, draw a short string (e.g. version/status text), and flush —
  replacing (or following) the all-on test pattern as a more useful smoke
  test.

## Capabilities

### New Capabilities
- `ssd1309-framebuffer`: RAM framebuffer, pixel-level drawing primitives,
  full-buffer flush to GDDRAM, and basic monospace text rendering for the
  SSD1309 driver.

### Modified Capabilities
(none — `ssd1309-display-bringup` is unchanged; `ssd1309_bringup()` remains
the one-time power-on/init entry point, framebuffer operations are
additional driver functions used after bring-up)

## Impact

- `Core/Inc/ssd1309.h`, `Core/Src/ssd1309.c` — add framebuffer storage to
  `ssd1309_t` (or a new struct), drawing/flush/text functions, and font
  table.
- `Core/Src/app.c` — after `ssd1309_bringup()`, exercise the new
  clear/draw_string/flush API as the bench bring-up smoke test.
- I2C1 bus — `ssd1309_flush()` becomes the steady-state way display content
  reaches the panel (still blocking, called from `app_init()`/`app_loop()`
  per the cooperative-scheduler pattern).
