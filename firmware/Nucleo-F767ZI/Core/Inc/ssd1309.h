
#ifndef INC_SSD1309_H_
#define INC_SSD1309_H_

#include "main.h"
#include <stdint.h>

typedef struct {
	I2C_HandleTypeDef *port;
	uint16_t address;
	int16_t height;
	int16_t width;
} ssd1309_t;


/**
 * ssd1309_init
 * @param p - Pointer to ssd1309_t struct
 * @param in_port - Pointer to I2C_HandleTypeDef
 * @param in_address - The 7-bit I2C address
 * @param in_height - The  display pixel height (-1 defaults to 64)
 * @param in_width - The  display pixel width (-1 defaults to 128)
 *
 * Populates the handle only - no I2C traffic is sent.
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

#endif /* INC_SSD1309_H_ */
