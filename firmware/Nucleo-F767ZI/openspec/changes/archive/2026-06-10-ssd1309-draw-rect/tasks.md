## 1. Rectangle drawing implementation

- [x] 1.1 Implement `void ssd1309_draw_rect(ssd1309_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool fill, uint8_t color)`
      in `Core/Src/ssd1309.c`: normalize corners to
      `(min_x,min_y)`-`(max_x,max_y)`, then either draw the four edges with
      `ssd1309_draw_line()` (`fill == false`) or set every pixel in the
      rectangle with `ssd1309_set_pixel()` (`fill == true`)
- [x] 1.2 Declare `ssd1309_draw_rect()` in `Core/Inc/ssd1309.h`

## 2. Sandbox integration

- [x] 2.1 In `Core/Src/app.c` `app_init()`, draw a demo rectangle (e.g. an
      outline box and/or a small filled box) and call `ssd1309_flush()`

## 3. Bench verification

- [x] 3.1 Build and flash via the external ST-LINK V3, confirm both outline
      and filled rectangles render correctly on the OLED panel
