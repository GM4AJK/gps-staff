
#ifndef INC_BNO085_H_
#define INC_BNO085_H_

#include "main.h"
#include <stdint.h>

/* Reset pulse width (RST held low) */
#define BNO085_RESET_PULSE_MS 10

/* Max time to wait for INT to go low (data ready) after reset */
#define BNO085_INT_TIMEOUT_MS 1000

/* Max time to wait for INT to read high (deasserted) after releasing RST,
 * before polling for it to go low. Guards against an immediate-low INT
 * reading right after reset, which indicates the chip is still in
 * reset/boot rather than signaling data-ready. */
#define BNO085_INT_DEASSERT_TIMEOUT_MS 50

/* Size of the single SPI read performed by bno085_bringup() */
#define BNO085_BRINGUP_BUF_SIZE 32

/* Size of the single SPI read performed by bno085_read_advertisement() */
#define BNO085_ADVERT_BUF_SIZE 320

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
	uint8_t advert_buf[BNO085_ADVERT_BUF_SIZE];
	uint16_t advert_len;
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

/**
 * bno085_read_advertisement
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Performs the same RST pulse and INT-wait sequence as bno085_bringup(),
 * recording int_initial and int_wait_ms. On success, performs a single
 * BNO085_ADVERT_BUF_SIZE-byte full-duplex SPI transfer (transmitting
 * all-zero bytes) with CS asserted, then releases CS, parses the first 4
 * received bytes as the SHTP header, and stores
 * advert_len = min(shtp_length, BNO085_ADVERT_BUF_SIZE). The full received
 * buffer is stored in advert_buf.
 *
 * @return HAL_OK on success, HAL_TIMEOUT if INT does not go low in time, or
 *         the HAL_StatusTypeDef of a failed SPI transfer.
 */
HAL_StatusTypeDef bno085_read_advertisement(bno085_t *p);

/**
 * bno085_print_advertisement
 * @param p - Pointer to a bno085_t struct populated by a successful call to
 *            bno085_read_advertisement()
 * @param huart - UART handle to print the parsed records to
 *
 * Walks advert_buf[4 .. advert_len) as a sequence of tag/length/value
 * records (1 byte tag, 1 byte length, length bytes of value), and
 * transmits one line per record over huart:
 *  - length == 0: the tag only
 *  - value is all printable ASCII: the tag and value as a quoted string
 *  - length == 1 (and not printable ASCII): the tag and value as a decimal
 *    number
 *  - otherwise: the tag and value as space-separated hex bytes
 *
 * The walk stops before reading any record that would extend past
 * advert_len or BNO085_ADVERT_BUF_SIZE.
 */
void bno085_print_advertisement(bno085_t *p, UART_HandleTypeDef *huart);

#endif /* INC_BNO085_H_ */
