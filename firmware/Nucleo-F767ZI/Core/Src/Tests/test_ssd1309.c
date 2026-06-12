
#include "Tests/test_ssd1309.h"

#ifdef TEST_SSD1309

HAL_StatusTypeDef test_ssd1309_shapes(ssd1309_t *p)
{
	ssd1309_clear(p);
	ssd1309_draw_string(p, &font5x7, 0, 0, "Nucleo-F767ZI OK", SSD1309_COLOR_ON);
	ssd1309_draw_line(p, 0, 9, p->width - 1, 9, SSD1309_COLOR_ON);
	ssd1309_draw_rect(p, 0, 14, 20, 34, false, SSD1309_COLOR_ON);
	ssd1309_draw_rect(p, 26, 14, 46, 34, true, SSD1309_COLOR_ON);
	ssd1309_draw_circle(p, 75, 24, 10, false, SSD1309_COLOR_ON);
	ssd1309_draw_circle(p, 105, 24, 10, true, SSD1309_COLOR_ON);
	ssd1309_draw_arrow(p, 10, 45, 50, 60, 6, SSD1309_COLOR_ON);
	ssd1309_draw_triangle(p, 70, 45, 90, 45, 80, 62, false, SSD1309_COLOR_ON);

	static const ssd1309_point_t pentagon[] = {
		{ 59, 45 },
		{ 67, 51 },
		{ 64, 60 },
		{ 54, 60 },
		{ 51, 51 },
	};
	ssd1309_draw_polygon(p, pentagon, 5, true, SSD1309_COLOR_ON);

	static const ssd1309_point_t hexagon[] = {
		{ 105, 46 },
		{ 112, 50 },
		{ 112, 58 },
		{ 105, 62 },
		{ 98, 58 },
		{ 98, 50 },
	};
	ssd1309_draw_polygon(p, hexagon, 6, false, SSD1309_COLOR_ON);

	return ssd1309_flush(p);
}

#endif /* TEST_SSD1309 */
