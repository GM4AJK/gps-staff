
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

/* Size of the command buffer used by bno085_get_feature() */
#define BNO085_CMD_BUF_SIZE 32

/* SHTP channel numbers, from the parsed advertisement */
#define BNO085_CHANNEL_CONTROL 2

/* SH-2 report IDs for the Get Feature Request/Response pair */
#define BNO085_REPORT_ID_GET_FEATURE_REQUEST 0xFE
#define BNO085_REPORT_ID_GET_FEATURE_RESPONSE 0xFC

/* Number of SHTP channels for which a host TX sequence number is tracked */
#define BNO085_NUM_CHANNELS 6

/* Max number of packets bno085_get_feature() will read (discarding
 * non-matching ones) while looking for its Get Feature Response */
#define BNO085_GET_FEATURE_MAX_ATTEMPTS 4

typedef struct {
	uint8_t feature_report_id;
	uint8_t feature_flags;
	uint16_t change_sensitivity;
	uint32_t report_interval_us;
	uint32_t batch_interval_us;
	uint32_t sensor_specific_config;
} bno085_feature_t;

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
	uint8_t tx_seq[BNO085_NUM_CHANNELS];
	uint8_t cmd_buf[BNO085_CMD_BUF_SIZE];
	uint16_t cmd_len;
	bno085_feature_t feature;
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
 * recording int_initial and int_wait_ms. On success, asserts CS, performs a
 * 4-byte full-duplex SPI transfer (transmitting all-zero bytes) to read the
 * SHTP header, computes advert_len = min(shtp_length, BNO085_ADVERT_BUF_SIZE)
 * from it, then performs a second full-duplex SPI transfer of exactly
 * (advert_len - 4) more bytes (if any) before releasing CS. Reading exactly
 * advert_len bytes (rather than always BNO085_ADVERT_BUF_SIZE) avoids
 * draining bytes from whatever packet the device queues next. advert_buf[0
 * .. advert_len) contains the received header and payload; bytes beyond
 * advert_len are not written by this call.
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

/**
 * bno085_send_packet
 * @param p - Pointer to an initialized bno085_t struct
 * @param channel - SHTP channel number to send on (0..BNO085_NUM_CHANNELS-1)
 * @param payload - Pointer to the payload bytes to send
 * @param payload_len - Number of payload bytes
 *
 * Builds a 4-byte SHTP header (length = 4 + payload_len, channel, and the
 * channel's current host TX sequence number from tx_seq[]), then asserts CS
 * and performs a full-duplex SPI transfer of the header followed by the
 * payload. SPI is full-duplex, so the device may simultaneously be sending a
 * queued packet of its own; the bytes received during this transfer are
 * captured into cmd_buf rather than discarded (so they aren't lost from the
 * device's queue), and cmd_len is set from the received SHTP header's length
 * field (capped to BNO085_CMD_BUF_SIZE). The transfer is exactly
 * max(4 + payload_len, cmd_len) bytes, so neither side's packet is
 * under-clocked. CS is released afterwards. On success, tx_seq[channel] is
 * incremented (with uint8_t wraparound).
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of a failed SPI
 *         transfer (CS is released before returning either way).
 */
HAL_StatusTypeDef bno085_send_packet(bno085_t *p, uint8_t channel, const uint8_t *payload, uint16_t payload_len);

/**
 * bno085_get_feature
 * @param p - Pointer to an initialized bno085_t struct
 * @param report_id - SH-2 report ID of the sensor to query (e.g. 0x01 for
 *                     Accelerometer)
 *
 * Sends an SH-2 Get Feature Request for report_id on BNO085_CHANNEL_CONTROL
 * via bno085_send_packet(), which captures whatever packet the device had
 * queued into cmd_buf/cmd_len as a side effect of the write. If that packet
 * is at least 21 bytes with a payload of Get Feature Response (0xFC)
 * followed by report_id, it is used directly. Otherwise, up to
 * BNO085_GET_FEATURE_MAX_ATTEMPTS - 1 more times: waits for INT to go low
 * within BNO085_INT_TIMEOUT_MS, then reads the 4-byte SHTP header followed by
 * exactly (length - 4) more bytes (capped to BNO085_CMD_BUF_SIZE) into
 * cmd_buf/cmd_len - see bno085_read_advertisement() for why the read length
 * must match the packet's declared length exactly. Each non-matching packet
 * is discarded and another is read. Once a matching packet is found, parses
 * the remaining bytes little-endian into the feature field and returns.
 *
 * @return HAL_OK on a matching Get Feature Response, HAL_TIMEOUT if INT does
 *         not go low in time on a retry read, HAL_ERROR if no matching
 *         response is found within BNO085_GET_FEATURE_MAX_ATTEMPTS packets,
 *         or the HAL_StatusTypeDef of a failed SPI transfer.
 */
HAL_StatusTypeDef bno085_get_feature(bno085_t *p, uint8_t report_id);

#endif /* INC_BNO085_H_ */
