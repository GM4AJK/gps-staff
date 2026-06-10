
#ifndef INC_SSD1309_H_
#define INC_SSD1309_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Max framebuffer size: 128 cols x 8 pages (128x64 panel, 1bpp) */
#define SSD1309_FB_SIZE (128u * 64u / 8u)

#define SSD1309_COLOR_OFF 0
#define SSD1309_COLOR_ON  1

typedef struct {
	I2C_HandleTypeDef *port;
	uint16_t address;
	int16_t height;
	int16_t width;
	uint8_t buffer[SSD1309_FB_SIZE];
	bool dirty;
} ssd1309_t;


/**
 * ssd1309_init
 * @param p - Pointer to ssd1309_t struct
 * @param in_port - Pointer to I2C_HandleTypeDef
 * @param in_address - The 7-bit I2C address
 * @param in_height - The  display pixel height (-1 defaults to 64)
 * @param in_width - The  display pixel width (-1 defaults to 128)
 *
 * Populates the handle and clears the framebuffer - no I2C traffic is sent.
 */
void ssd1309_init(
	ssd1309_t *p,
	I2C_HandleTypeDef *in_port,
	uint16_t in_address,
	int16_t in_height,
	int16_t in_width
);

/**
 * ssd1309_bringup
 * @param p - Pointer to an initialized ssd1309_t struct
 *
 * Sends the SSD1309 power-on init command sequence followed by an
 * all-pixels-on test pattern, then turns the display on. Intended as a
 * bench bring-up / wiring smoke test.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the first failed
 *         I2C transfer.
 */
HAL_StatusTypeDef ssd1309_bringup(ssd1309_t *p);

/**
 * ssd1309_clear
 * @param p - Pointer to ssd1309_t struct
 *
 * Sets every pixel in the framebuffer off and marks it dirty.
 */
void ssd1309_clear(ssd1309_t *p);

/**
 * ssd1309_fill
 * @param p - Pointer to ssd1309_t struct
 * @param color - SSD1309_COLOR_OFF or SSD1309_COLOR_ON
 *
 * Sets every pixel in the framebuffer to color and marks it dirty.
 */
void ssd1309_fill(ssd1309_t *p, uint8_t color);

/**
 * ssd1309_set_pixel
 * @param p - Pointer to ssd1309_t struct
 * @param x - Column (0 .. width-1)
 * @param y - Row (0 .. height-1)
 * @param color - SSD1309_COLOR_OFF or SSD1309_COLOR_ON
 *
 * Out-of-bounds coordinates are silently ignored. Marks the framebuffer
 * dirty only if the pixel's value actually changes.
 */
void ssd1309_set_pixel(ssd1309_t *p, int16_t x, int16_t y, uint8_t color);

/**
 * ssd1309_get_pixel
 * @param p - Pointer to ssd1309_t struct
 * @param x - Column (0 .. width-1)
 * @param y - Row (0 .. height-1)
 *
 * @return SSD1309_COLOR_ON or SSD1309_COLOR_OFF. Out-of-bounds coordinates
 *         return SSD1309_COLOR_OFF.
 */
uint8_t ssd1309_get_pixel(ssd1309_t *p, int16_t x, int16_t y);

/**
 * ssd1309_flush
 * @param p - Pointer to an initialized ssd1309_t struct, after a
 *            successful ssd1309_bringup()
 *
 * Writes the framebuffer to GDDRAM via page-addressing mode if the
 * framebuffer is dirty; otherwise a no-op.
 *
 * @return HAL_OK on success or if not dirty, or the HAL_StatusTypeDef of
 *         the first failed I2C transfer (dirty flag is left set).
 */
HAL_StatusTypeDef ssd1309_flush(ssd1309_t *p);

/**
 * ssd1309_draw_char
 * @param p - Pointer to ssd1309_t struct
 * @param x - Column of the glyph's top-left pixel
 * @param y - Row of the glyph's top-left pixel
 * @param c - Character to draw (0x20 .. 0x7E); other values render blank
 * @param color - SSD1309_COLOR_OFF or SSD1309_COLOR_ON
 *
 * Draws a 5x7 glyph from the built-in font using ssd1309_set_pixel().
 */
void ssd1309_draw_char(ssd1309_t *p, int16_t x, int16_t y, char c, uint8_t color);

/**
 * ssd1309_draw_string
 * @param p - Pointer to ssd1309_t struct
 * @param x - Column of the first glyph's top-left pixel
 * @param y - Row of the first glyph's top-left pixel
 * @param str - NUL-terminated string to draw
 * @param color - SSD1309_COLOR_OFF or SSD1309_COLOR_ON
 *
 * Draws each character via ssd1309_draw_char(), advancing x by 6px per
 * character. No wrapping is performed.
 */
void ssd1309_draw_string(ssd1309_t *p, int16_t x, int16_t y, const char *str, uint8_t color);

#endif /* INC_SSD1309_H_ */
