## 1. Arrow drawing implementation

- [x] 1.1 Implement `void ssd1309_draw_arrow(ssd1309_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t size, uint8_t color)`
      in `Core/Src/ssd1309.c`: draw the shaft via `ssd1309_draw_line()`, then
      (if `size > 0`) compute `angle = atan2f(y1-y0, x1-x0)` and draw two
      wing lines from `(x1, y1)` back along `angle ± 30°` of length `size`,
      via `ssd1309_draw_line()`
- [x] 1.2 Declare `ssd1309_draw_arrow()` in `Core/Inc/ssd1309.h`

## 2. Sandbox integration

- [x] 2.1 In `Core/Src/app.c` `app_init()`, draw a demo arrow (e.g. at an
      angle, not purely horizontal/vertical, to exercise the general case)
      and call `ssd1309_flush()`

## 3. Bench verification

- [x] 3.1 Build and flash via the external ST-LINK V3, confirm the arrow
      (shaft + arrowhead) renders correctly and points in the expected
      direction on the OLED panel
