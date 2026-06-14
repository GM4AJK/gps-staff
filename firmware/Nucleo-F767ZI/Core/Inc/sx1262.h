
#ifndef INC_SX1262_H_
#define INC_SX1262_H_

#include "main.h"
#include <stdint.h>

/* GetStatus (datasheet 13.5.1) */
#define SX1262_OP_GET_STATUS 0xC0
#define SX1262_OP_NOP        0x00

#define SX1262_SPI_TIMEOUT_MS  100
#define SX1262_BUSY_TIMEOUT_MS 1000

typedef struct {
	SPI_HandleTypeDef *port;
	GPIO_PIN_DEF(cs_port, cs_pin);
	GPIO_PIN_DEF(reset_port, reset_pin);
	GPIO_PIN_DEF(busy_port, busy_pin);
} sx1262_t;

/**
 * sx1262_init
 * @param p - Pointer to an sx1262_t struct to initialize
 * @param in_port - SPI handle the SX1262 is wired to
 * @param cs_port, cs_pin - NSS (chip select) GPIO, active low
 * @param reset_port, reset_pin - NRESET GPIO, active low
 * @param busy_port, busy_pin - BUSY GPIO (input)
 *
 * Stores the SPI handle and GPIO pins used to talk to the SX1262. Does not
 * touch any pins or perform any SPI transactions.
 */
void sx1262_init(
	sx1262_t *p,
	SPI_HandleTypeDef *in_port,
	GPIO_TypeDef *cs_port, uint16_t cs_pin,
	GPIO_TypeDef *reset_port, uint16_t reset_pin,
	GPIO_TypeDef *busy_port, uint16_t busy_pin
);

/**
 * sx1262_wait_busy
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Polls the BUSY line until it reads low, or SX1262_BUSY_TIMEOUT_MS
 * elapses. The SX1262 drives BUSY high while processing a command and
 * while booting after reset; SPI transactions must not be started while
 * BUSY is high.
 *
 * @return HAL_OK once BUSY is low, or HAL_TIMEOUT if it does not clear in
 *         time.
 */
HAL_StatusTypeDef sx1262_wait_busy(sx1262_t *p);

/**
 * sx1262_reset
 * @param p - Pointer to an initialized sx1262_t struct
 *
 * Pulses NRESET low then high, then waits for BUSY to clear as the SX1262
 * completes its boot sequence.
 *
 * @return HAL_OK once the SX1262 is ready, or HAL_TIMEOUT if BUSY does not
 *         clear in time.
 */
HAL_StatusTypeDef sx1262_reset(sx1262_t *p);

/**
 * sx1262_get_status
 * @param p - Pointer to an initialized sx1262_t struct
 * @param out_status - Receives the status byte returned by the SX1262
 *
 * Sends the GetStatus (0xC0) command over SPI and reads back the status
 * byte. Bench test for basic SPI framing, NSS and BUSY handling.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed step.
 */
HAL_StatusTypeDef sx1262_get_status(sx1262_t *p, uint8_t *out_status);

#endif /* INC_SX1262_H_ */
