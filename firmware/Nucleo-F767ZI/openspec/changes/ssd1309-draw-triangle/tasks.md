## 1. Triangle drawing implementation

- [x] 1.1 Implement `void ssd1309_draw_triangle(ssd1309_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, uint8_t color)`
      in `Core/Src/ssd1309.c`:
      for `fill == false`, draw the three edges via `ssd1309_draw_line()`;
      for `fill == true`, sort vertices by `y`, then for each scanline draw
      a horizontal span via `ssd1309_draw_line()` between the
      x-intersections of the flat-bottom/flat-top edge pairs (standard
      scanline fill)
- [x] 1.2 Declare `ssd1309_draw_triangle()` in `Core/Inc/ssd1309.h`

## 2. Sandbox integration

- [x] 2.1 In `Core/Src/app.c` `app_init()`, draw a demo triangle (outline
      and/or filled) and call `ssd1309_flush()`

## 3. Bench verification

- [x] 3.1 Build and flash via the external ST-LINK V3, confirm both outline
      and filled triangles render correctly on the OLED panel
