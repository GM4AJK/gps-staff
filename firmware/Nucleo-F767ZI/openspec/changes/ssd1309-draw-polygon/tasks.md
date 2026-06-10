## 1. Polygon drawing implementation

- [x] 1.1 Add `ssd1309_point_t { int16_t x; int16_t y; }` struct and
      `SSD1309_POLYGON_MAX_POINTS` (32) define to `Core/Inc/ssd1309.h`
- [x] 1.2 Implement `void ssd1309_draw_polygon(ssd1309_t *p, const ssd1309_point_t *points, uint8_t num_points, bool fill, uint8_t color)`
      in `Core/Src/ssd1309.c`:
      for `fill == false`, draw an edge between each consecutive pair of
      points plus the closing edge, via `ssd1309_draw_line()`; for
      `fill == true`, use an even-odd scanline fill (per scanline, collect
      x-intersections with each edge into a fixed-size buffer, sort, and
      draw spans via `ssd1309_draw_line()` for each pair)
- [x] 1.3 Declare `ssd1309_draw_polygon()` in `Core/Inc/ssd1309.h`

## 2. Sandbox integration

- [x] 2.1 In `Core/Src/app.c` `app_init()`, draw a demo polygon (outline
      and/or filled, with more than 3 vertices) and call
      `ssd1309_flush()`

## 3. Bench verification

- [x] 3.1 Build and flash via the external ST-LINK V3, confirm both outline
      and filled polygons render correctly on the OLED panel
