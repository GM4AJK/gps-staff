## 1. Circle drawing implementation

- [x] 1.1 Implement `void ssd1309_draw_circle(ssd1309_t *p, int16_t x0, int16_t y0, int16_t r, bool fill, uint8_t color)`
      in `Core/Src/ssd1309.c` using the integer midpoint circle algorithm:
      for `fill == false`, plot the 8 symmetric points per step via
      `ssd1309_set_pixel()`; for `fill == true`, draw horizontal spans per
      step via `ssd1309_draw_line()`; `r == 0` draws a single point
- [x] 1.2 Declare `ssd1309_draw_circle()` in `Core/Inc/ssd1309.h`

## 2. Sandbox integration

- [x] 2.1 In `Core/Src/app.c` `app_init()`, draw a demo circle (e.g. an
      outline circle and/or a small filled circle) and call
      `ssd1309_flush()`

## 3. Bench verification

- [x] 3.1 Build and flash via the external ST-LINK V3, confirm both outline
      and filled circles render correctly on the OLED panel
