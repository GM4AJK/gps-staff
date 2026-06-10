
#ifndef INC_BNO085_H_
#define INC_BNO085_H_

#include "main.h"
#include <stdint.h>

/* Reset pulse width (RST held low) */
#define BNO085_RESET_PULSE_MS 10

/* Max time to wait for INT to go low (data ready) after reset */
#define BNO085_INT_TIMEOUT_MS 1000

/* Size of the single SPI read performed by bno085_bringup() */
#define BNO085_BRINGUP_BUF_SIZE 32

typedef struct {
	SPI_HandleTypeDef *port;
	GPIO_TypeDef *cs_port;
	uint16_t cs_pin;
	GPIO_TypeDef *rst_port;
	uint16_t rst_pin;
	GPIO_TypeDef *int_port;
	uint16_t int_pin;
	uint8_t rx_buf[BNO085_BRINGUP_BUF_SIZE];
	uint16_t shtp_length;
	uint8_t shtp_channel;
	uint8_t shtp_sequence;
	GPIO_PinState int_initial;
	uint32_t int_wait_ms;
} bno085_t;

/**
 * bno085_init
 * @param p - Pointer to bno085_t struct
 * @param in_port - Pointer to SPI_HandleTypeDef
 * @param cs_port - GPIO port for the CS pin
 * @param cs_pin - GPIO pin for the CS pin
 * @param rst_port - GPIO port for the RST pin
 * @param rst_pin - GPIO pin for the RST pin
 * @param int_port - GPIO port for the INT pin
 * @param int_pin - GPIO pin for the INT pin
 *
 * Populates the handle - no SPI traffic or GPIO writes are performed.
 */
void bno085_init(
	bno085_t *p,
	SPI_HandleTypeDef *in_port,
	GPIO_TypeDef *cs_port,
	uint16_t cs_pin,
	GPIO_TypeDef *rst_port,
	uint16_t rst_pin,
	GPIO_TypeDef *int_port,
	uint16_t int_pin
);

/**
 * bno085_bringup
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Drives RST low for BNO085_RESET_PULSE_MS then high, then polls INT for
 * it to go low (data ready) within BNO085_INT_TIMEOUT_MS, recording the
 * pre-reset INT level in int_initial and the time waited in int_wait_ms.
 * On success,
 * performs a single BNO085_BRINGUP_BUF_SIZE-byte full-duplex SPI transfer
 * (transmitting all-zero bytes) with CS asserted, then releases CS and
 * parses the first 4 received bytes as the SHTP header into
 * shtp_length/shtp_channel/shtp_sequence.
 *
 * @return HAL_OK on success, HAL_TIMEOUT if INT does not go low in time, or
 *         the HAL_StatusTypeDef of a failed SPI transfer.
 */
HAL_StatusTypeDef bno085_bringup(bno085_t *p);

#endif /* INC_BNO085_H_ */
