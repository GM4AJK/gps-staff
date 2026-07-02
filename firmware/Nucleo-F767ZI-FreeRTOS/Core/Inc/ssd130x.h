
#ifndef INC_SSD130X_H_
#define INC_SSD130X_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Max framebuffer size: 128 cols x 8 pages (128x64 panel, 1bpp) */
#define SSD130X_FB_SIZE (128u * 64u / 8u)

#define SSD130X_COLOR_OFF 0
#define SSD130X_COLOR_ON  1

/* Max vertices supported by ssd130x_draw_polygon() */
#define SSD130X_POLYGON_MAX_POINTS 32

typedef enum {
	SSD130X_CHIP_SSD1306,
	SSD130X_CHIP_SSD1309,
} ssd130x_chip_t;

typedef struct {
	I2C_HandleTypeDef *port;
	uint16_t address;
	int16_t height;
	int16_t width;
	ssd130x_chip_t chip;
	bool rotate_180;
	uint8_t buffer[SSD130X_FB_SIZE];
	bool dirty;
} ssd130x_t;

typedef struct {
	int16_t x;
	int16_t y;
} ssd130x_point_t;

typedef struct {
	const uint8_t *glyphs;
	uint8_t glyph_width;
	uint8_t glyph_height;
	uint8_t advance;
	char first_char;
	char last_char;
} ssd130x_font_t;

/* Built-in fonts, all covering printable ASCII 0x20-0x7E */
extern const ssd130x_font_t ssd130x_font5x7;
extern const ssd130x_font_t ssd130x_font8x8;
extern const ssd130x_font_t ssd130x_font10x14;

void ssd130x_init(
	ssd130x_t *p,
	I2C_HandleTypeDef *in_port,
	uint16_t in_address,
	ssd130x_chip_t chip,
	bool rotate_180,
	int16_t in_height,
	int16_t in_width
);

HAL_StatusTypeDef ssd130x_bringup(ssd130x_t *p);
void ssd130x_clear(ssd130x_t *p);
void ssd130x_fill(ssd130x_t *p, uint8_t color);
void ssd130x_set_pixel(ssd130x_t *p, int16_t x, int16_t y, uint8_t color);
uint8_t ssd130x_get_pixel(ssd130x_t *p, int16_t x, int16_t y);
HAL_StatusTypeDef ssd130x_flush(ssd130x_t *p);
void ssd130x_draw_line(ssd130x_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t color);
void ssd130x_draw_rect(ssd130x_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, bool fill, uint8_t color);
void ssd130x_draw_circle(ssd130x_t *p, int16_t x0, int16_t y0, int16_t r, bool fill, uint8_t color);
void ssd130x_draw_polygon(ssd130x_t *p, const ssd130x_point_t *points, uint8_t num_points, bool fill, uint8_t color);
void ssd130x_draw_triangle(ssd130x_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool fill, uint8_t color);
void ssd130x_draw_arrow(ssd130x_t *p, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t size, uint8_t color);
void ssd130x_draw_char(ssd130x_t *p, const ssd130x_font_t *font, int16_t x, int16_t y, char c, uint8_t color);
void ssd130x_draw_string(ssd130x_t *p, const ssd130x_font_t *font, int16_t x, int16_t y, const char *str, uint8_t color);

#endif /* INC_SSD130X_H_ */
