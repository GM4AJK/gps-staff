
#ifndef INC_SSD1309_H_
#define INC_SSD1309_H_

#include "main.h"
#include <stdint.h>

typedef struct {
	I2C_HandleTypeDef port;
	uint16_t address;
	int16_t height;
	int16_t width;
} ssd1309_t;


/**
 * ssd1309_init
 * @param p - Pointer to ssd1309_t struct
 * @param in_port - I2C_HandleTypeDef
 * @param in_address - The I2C address
 * @param in_height - The  display pixel height (-1 defaults to 64)
 * @param in_width - The  display pixel width (-1 defaults to 128)
 */
void ssd1309_init(
	ssd1309_t *p,
	I2C_HandleTypeDef in_port,
	uint16_t in_address,
	int16_t in_height,
	int16_t in_width
);


#endif /* INC_SSD1309_H_ */
