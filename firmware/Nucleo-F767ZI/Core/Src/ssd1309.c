

#include "ssd1309.h"

void ssd1309_init(
	ssd1309_t *p,
	I2C_HandleTypeDef in_port,
	uint16_t in_address,
	int16_t in_height,
	int16_t in_width)
{
	p->port = in_port;
	p->address = in_address;
	p->height = (in_height < 0) ? 64 : in_height;
	p->width = (in_width < 0) ? 128 : in_width;
}

