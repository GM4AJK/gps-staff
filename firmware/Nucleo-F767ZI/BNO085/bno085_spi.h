/*
 * bno085_spi.h
 *
 *  Created on: 11 Jun 2026
 *      Author: kirkh
 */

#ifndef INC_BNO085_SPI_H_
#define INC_BNO085_SPI_H_
#if 0

#include <stdbool.h>

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_tim.h"
#include "stm32f7xx_hal_spi.h"

#include "sh2_hal.h"
#include "sh2_err.h"

typedef enum SpiState_e
{
    SPI_INIT,
    SPI_DUMMY,
    SPI_DFU,
    SPI_IDLE,
    SPI_RD_HDR,
    SPI_RD_BODY,
    SPI_WRITE
} SpiState_t;

typedef struct
{
	GPIO_TypeDef *port;
	uint16_t pin;
}
bno085_gpio_t;

typedef struct
{
	SPI_HandleTypeDef *spi;
	bno085_gpio_t chip_select;
	bno085_gpio_t ps0_wake;
	bno085_gpio_t interrupt;
	bno085_gpio_t reset;
	SpiState_t spiState;
	void (*us_delay_fp)(uint32_t);
	uint8_t seq_number[5];
	bool data_available;
	uint8_t shtp_header[4];
	uint8_t shtp_data[512];
} bno085_t;

// API function calls

HAL_StatusTypeDef bno085_init(bno085_t *p);

bool bno085_enable_rotation_vector(bno085_t *p, uint16_t t_between);
bool bno085_enableGameRotationVector(bno085_t *p, uint16_t t_between);
bool bno085_enableAccelerometer(bno085_t *p, uint16_t t_between);
bool bno085_enableGyro(bno085_t *p, uint16_t t_between);
bool bno085_enableMagnetometer(bno085_t *p, uint16_t t_between);
bool bno085_enableLinearAccelerometer(bno085_t *p, uint16_t t_between);
bool bno085_enableGravity(bno085_t *p, uint16_t t_between);

#endif /* INC_BNO085_SPI_H_ */
#endif
