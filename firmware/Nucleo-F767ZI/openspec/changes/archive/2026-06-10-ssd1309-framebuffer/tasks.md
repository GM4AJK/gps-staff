## 1. Framebuffer storage

- [x] 1.1 Add `uint8_t buffer[1024]` (128x64/8) to `ssd1309_t` in
      `Core/Inc/ssd1309.h`
- [x] 1.2 Add `bool dirty` field to `ssd1309_t`, initialized to `false` in
      `ssd1309_init()`
- [x] 1.3 Add `#define SSD1309_COLOR_OFF 0` / `#define SSD1309_COLOR_ON 1`
      constants

## 2. Pixel primitives

- [x] 2.1 Implement `void ssd1309_clear(ssd1309_t *p)` (memset buffer to 0,
      set `dirty = true`)
- [x] 2.2 Implement `void ssd1309_fill(ssd1309_t *p, uint8_t color)`
      (memset buffer to 0x00 or 0xFF based on color, set `dirty = true`)
- [x] 2.3 Implement `void ssd1309_set_pixel(ssd1309_t *p, int16_t x, int16_t y, uint8_t color)`
      with bounds checking against `p->width`/`p->height` (no-op if out of
      range); set `dirty = true` only if the pixel's value actually changes
- [x] 2.4 Implement `uint8_t ssd1309_get_pixel(ssd1309_t *p, int16_t x, int16_t y)`
      with bounds checking (returns `SSD1309_COLOR_OFF` if out of range)
- [x] 2.5 Declare all of the above in `Core/Inc/ssd1309.h`

## 3. Flush

- [x] 3.1 Extract a static helper `ssd1309_write_pages(ssd1309_t *p, const uint8_t *data)`
      in `Core/Src/ssd1309.c` that performs the per-page
      `0xB0+page / 0x00 / 0x10` command write + page-data write loop
- [x] 3.2 Update `ssd1309_bringup()` to call `ssd1309_write_pages()` for its
      all-`0xFF` test pattern (behavior unchanged, internal refactor only)
- [x] 3.3 Implement `HAL_StatusTypeDef ssd1309_flush(ssd1309_t *p)`: return
      `HAL_OK` immediately if `!p->dirty`; otherwise call
      `ssd1309_write_pages(p, p->buffer)`, and on `HAL_OK` set
      `dirty = false` (leave `dirty` set on failure)
- [x] 3.4 Declare `ssd1309_flush()` in `Core/Inc/ssd1309.h`

## 4. Font and text rendering

- [x] 4.1 Add `static const uint8_t font5x7[95][5]` covering printable ASCII
      `0x20`-`0x7E`, one byte per glyph column, bit 0 = top row (write a
      fresh bitmap, not copied from an existing library)
- [x] 4.2 Implement `void ssd1309_draw_char(ssd1309_t *p, int16_t x, int16_t y, char c, uint8_t color)`
      using `ssd1309_set_pixel()` for each glyph pixel; characters outside
      `0x20`-`0x7E` render blank
- [x] 4.3 Implement `void ssd1309_draw_string(ssd1309_t *p, int16_t x, int16_t y, const char *str, uint8_t color)`
      calling `ssd1309_draw_char()` per character, advancing `x` by 6px
- [x] 4.4 Declare `ssd1309_draw_char()` and `ssd1309_draw_string()` in
      `Core/Inc/ssd1309.h`

## 5. Sandbox integration

- [x] 5.1 In `Core/Src/app.c` `app_init()`, after `ssd1309_bringup()`
      succeeds, call `ssd1309_clear()`, `ssd1309_draw_string()` with a short
      status string (e.g. board/firmware name), and `ssd1309_flush()`
- [x] 5.2 On `HAL_StatusTypeDef` error from `ssd1309_flush()`, print a
      diagnostic message over USART3 (matching existing error-handling
      style)

## 6. Bench verification

- [ ] 6.1 Build and flash via the external ST-LINK V3, confirm the OLED
      panel displays the status string clearly and legibly
- [ ] 6.2 Confirm `ssd1309_clear()`/`ssd1309_fill()`/`ssd1309_flush()`
      produce the expected blank/full-on panel states (spot-check by
      temporarily calling each from `app_init()`)
