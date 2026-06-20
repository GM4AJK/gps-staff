#ifndef ILI9341_H
#define ILI9341_H

#include "stm32f7xx_hal.h"
#include <stdint.h>

#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320

/* RGB565 colour pack: R=5 bits, G=6 bits, B=5 bits */
#define ILI9341_RGB(r, g, b) \
	((uint16_t)(((r) & 0x1F) << 11) | (uint16_t)(((g) & 0x3F) << 5) | ((b) & 0x1F))

#define ILI9341_BLACK   0x0000
#define ILI9341_WHITE   0xFFFF
#define ILI9341_RED     0xF800
#define ILI9341_GREEN   0x07E0
#define ILI9341_BLUE    0x001F
#define ILI9341_YELLOW  0xFFE0
#define ILI9341_CYAN    0x07FF
#define ILI9341_MAGENTA 0xF81F

typedef struct {
	SPI_HandleTypeDef *spi;
	GPIO_TypeDef *cs_port;   uint16_t cs_pin;
	GPIO_TypeDef *dc_port;   uint16_t dc_pin;
	GPIO_TypeDef *rst_port;  uint16_t rst_pin;
	GPIO_TypeDef *bl_port;   uint16_t bl_pin;
	uint16_t width;
	uint16_t height;
} ili9341_t;

/*
 * ili9341_init
 * Populate the handle. No I/O is performed.
 */
void ili9341_init(ili9341_t *p,
	SPI_HandleTypeDef *spi,
	GPIO_TypeDef *cs_port,  uint16_t cs_pin,
	GPIO_TypeDef *dc_port,  uint16_t dc_pin,
	GPIO_TypeDef *rst_port, uint16_t rst_pin,
	GPIO_TypeDef *bl_port,  uint16_t bl_pin);

/*
 * ili9341_bringup
 * Hardware reset followed by the full ILI9341 init sequence.
 * Backlight is left off; call ili9341_backlight(p, 1) after.
 */
void ili9341_bringup(ili9341_t *p);

/*
 * ili9341_backlight
 * Turn the backlight on (1) or off (0).
 */
void ili9341_backlight(ili9341_t *p, uint8_t on);

/*
 * ili9341_fill
 * Fill the entire display with a single RGB565 colour.
 */
void ili9341_fill(ili9341_t *p, uint16_t colour);

/*
 * ili9341_fill_rect
 * Fill a rectangle (x, y, w, h) with a single RGB565 colour.
 */
void ili9341_fill_rect(ili9341_t *p,
	uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t colour);

/*
 * ili9341_draw_pixel
 * Draw a single pixel at (x, y).
 */
void ili9341_draw_pixel(ili9341_t *p, uint16_t x, uint16_t y, uint16_t colour);

/*
 * ili9341_write_pixels
 * Write a w*h block of pre-packed RGB565 pixels (big-endian, row-major).
 */
void ili9341_write_pixels(ili9341_t *p,
	uint16_t x, uint16_t y, uint16_t w, uint16_t h,
	const uint16_t *pixels, uint32_t n_pixels);

/*
 * ili9341_read_id
 * Read the 3-byte display ID (command 0x04). Returns the three ID bytes
 * in id[0..2]. The first dummy byte from the controller is discarded.
 * Requires MISO connected.
 */
void ili9341_read_id(ili9341_t *p, uint8_t id[3]);

#endif /* ILI9341_H */
