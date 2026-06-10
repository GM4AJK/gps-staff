## 1. Line drawing implementation

- [x] 1.1 Implement `void ssd1309_draw_line(ssd1309_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color)`
      in `Core/Src/ssd1309.c` using integer Bresenham's algorithm, calling
      `ssd1309_set_pixel()` for each point (handles all octants, both
      directions, and single-point lines)
- [x] 1.2 Declare `ssd1309_draw_line()` in `Core/Inc/ssd1309.h`

## 2. Sandbox integration

- [x] 2.1 In `Core/Src/app.c` `app_init()`, draw a short demo line (e.g. a
      horizontal separator under the status text) and call
      `ssd1309_flush()`

## 3. Bench verification

- [x] 3.1 Build and flash via the external ST-LINK V3, confirm the line
      renders correctly on the OLED panel alongside the existing status text
