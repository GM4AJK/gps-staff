
#ifndef INC_BNO085_H_
#define INC_BNO085_H_

#include "main.h"
#include <stdint.h>

/* Reset pulse width (RST held low) */
#define BNO085_RESET_PULSE_MS 10

/* Datasheet 6.5.3 "Startup timing": after releasing RST, INT is undefined
 * for t1 (Internal Initialization, 90ms typ) before the chip asserts INT
 * once t1+t2 (Internal configuration, 4ms max) have elapsed. Don't sample
 * INT until t1 has passed. */
#define BNO085_STARTUP_T1_MS 90

/* Max time to wait for INT to go low (data ready) after reset */
#define BNO085_INT_TIMEOUT_MS 5000

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
#define BNO085_CHANNEL_INPUT_REPORTS 3

/* SH-2 report IDs for the Get Feature Request/Response pair */
#define BNO085_REPORT_ID_GET_FEATURE_REQUEST 0xFE
#define BNO085_REPORT_ID_GET_FEATURE_RESPONSE 0xFC

/* SH-2 report ID for the Set Feature Command */
#define BNO085_REPORT_ID_SET_FEATURE_COMMAND 0xFD

/* SH-2 report IDs for sensor input reports handled by this driver */
#define BNO085_REPORT_ID_BASE_TIMESTAMP 0xFB
#define BNO085_REPORT_ID_ROTATION_VECTOR 0x05

/* Q point (fractional bits) of the Rotation Vector quaternion components
 * and accuracy estimate, per the SH-2 Reference Manual section 6.5.18 */
#define BNO085_ROTATION_VECTOR_Q_POINT 14
#define BNO085_ROTATION_VECTOR_ACCURACY_Q_POINT 12

/* SH-2 report IDs for the Command Request/Response pair */
#define BNO085_REPORT_ID_COMMAND_REQUEST 0xF2
#define BNO085_REPORT_ID_COMMAND_RESPONSE 0xF1

/* SH-2 command IDs used by this driver, sent via the Command Request */
#define BNO085_COMMAND_SAVE_DCD 0x06
#define BNO085_COMMAND_ME_CALIBRATION 0x07

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
	uint8_t sequence;
	uint8_t status;
	int16_t i;
	int16_t j;
	int16_t k;
	int16_t real;
	int16_t accuracy;
	float i_f;
	float j_f;
	float k_f;
	float real_f;
	float accuracy_rad;
} bno085_rotation_vector_t;

typedef struct {
	uint8_t status;
	uint8_t accel_enable;
	uint8_t gyro_enable;
	uint8_t mag_enable;
	uint8_t planar_accel_enable;
	uint8_t on_table_enable;
} bno085_me_calibration_t;

typedef struct {
	SPI_HandleTypeDef *port;
	GPIO_TypeDef *cs_port;
	uint16_t cs_pin;
	GPIO_TypeDef *rst_port;
	uint16_t rst_pin;
	GPIO_TypeDef *int_port;
	uint16_t int_pin;
	GPIO_TypeDef *wake_port;
	uint16_t wake_pin;
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
	uint8_t cmd_seq;
	uint8_t last_cmd_seq;
	bno085_feature_t feature;
	bno085_rotation_vector_t rotation_vector;
	bno085_me_calibration_t me_calibration;
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
 * @param wake_port - GPIO port for the PS0/WAKE pin
 * @param wake_pin - GPIO pin for the PS0/WAKE pin
 *
 * Populates the handle - no SPI traffic or GPIO writes are performed. The
 * caller's GPIO init must configure wake_pin as an output, initially driven
 * high (PS0/WAKE must be high from before reset until after the first
 * assertion of INT, to select the SPI interface).
 */
void bno085_init(
	bno085_t *p,
	SPI_HandleTypeDef *in_port,
	GPIO_TypeDef *cs_port,
	uint16_t cs_pin,
	GPIO_TypeDef *rst_port,
	uint16_t rst_pin,
	GPIO_TypeDef *int_port,
	uint16_t int_pin,
	GPIO_TypeDef *wake_port,
	uint16_t wake_pin
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
 * channel's current host TX sequence number from tx_seq[]). If INT is
 * already low, proceeds immediately; otherwise pulses PS0/WAKE low to ask
 * the device to assert INT and waits for it within BNO085_INT_TIMEOUT_MS
 * before returning PS0/WAKE high. Then asserts CS and performs a single
 * full-duplex SPI transfer of exactly (4 + payload_len) bytes (the header
 * followed by the payload), then releases CS. SPI is full-duplex, so the
 * device may simultaneously be sending a queued packet of its own; the bytes
 * received during this transfer are captured into cmd_buf (rather than
 * discarded) and cmd_len is set from the received SHTP header's length field
 * (capped to BNO085_CMD_BUF_SIZE) - this is for debugging visibility only, as
 * cmd_buf beyond the (4 + payload_len) bytes actually transferred is not
 * populated by this call. On success, tx_seq[channel] is incremented (with
 * uint8_t wraparound).
 *
 * @return HAL_OK on success, HAL_TIMEOUT if INT does not go low (after a
 *         PS0/WAKE pulse) within BNO085_INT_TIMEOUT_MS, or the
 *         HAL_StatusTypeDef of a failed SPI transfer (CS is released before
 *         returning either way).
 */
HAL_StatusTypeDef bno085_send_packet(bno085_t *p, uint8_t channel, const uint8_t *payload, uint16_t payload_len);

/**
 * bno085_get_feature
 * @param p - Pointer to an initialized bno085_t struct
 * @param report_id - SH-2 report ID of the sensor to query (e.g. 0x01 for
 *                     Accelerometer)
 *
 * Sends an SH-2 Get Feature Request for report_id on BNO085_CHANNEL_CONTROL
 * via bno085_send_packet(). Then, up to BNO085_GET_FEATURE_MAX_ATTEMPTS
 * times: wakes the device (if needed) and waits for INT to go low within
 * BNO085_INT_TIMEOUT_MS (see bno085_send_packet()), then reads the 4-byte
 * SHTP header followed by exactly (length - 4) more bytes (capped to
 * BNO085_CMD_BUF_SIZE) into cmd_buf/cmd_len - see bno085_read_advertisement()
 * for why the read length must match the packet's declared length exactly.
 * The response to our request is not always the next packet read (a
 * previously-queued packet, e.g. the response to an earlier Get Feature
 * Request, may be read first), so each non-matching packet is discarded and
 * another is read. If a packet is at least 21 bytes and its payload is a Get
 * Feature Response (0xFC) for report_id, parses the remaining bytes
 * little-endian into the feature field and returns.
 *
 * @return HAL_OK on a matching Get Feature Response, HAL_TIMEOUT if INT does
 *         not go low in time on a read, HAL_ERROR if no matching response is
 *         found within BNO085_GET_FEATURE_MAX_ATTEMPTS packets, or the
 *         HAL_StatusTypeDef of a failed SPI transfer.
 */
HAL_StatusTypeDef bno085_get_feature(bno085_t *p, uint8_t report_id);

/**
 * bno085_set_feature
 * @param p - Pointer to an initialized bno085_t struct
 * @param report_id - SH-2 report ID of the sensor to configure (e.g. 0x05
 *                     for Rotation Vector)
 * @param feature_flags - Feature flags byte (see SH-2 Set Feature Command)
 * @param change_sensitivity - Change sensitivity (units/Q point are
 *                              report-specific)
 * @param report_interval_us - Desired report interval, in microseconds
 * @param batch_interval_us - Desired batch interval, in microseconds
 * @param sensor_specific_config - Sensor-specific configuration word
 *
 * Builds a 17-byte SH-2 Set Feature Command (0xFD) payload - report_id,
 * feature_flags, change_sensitivity (2 bytes LE), report_interval_us
 * (4 bytes LE), batch_interval_us (4 bytes LE), and
 * sensor_specific_config (4 bytes LE) - and sends it on
 * BNO085_CHANNEL_CONTROL via bno085_send_packet(). Unlike
 * bno085_get_feature(), no response is awaited: the SH-2 Get Feature
 * Response is only sent unsolicited on a later rate change, not
 * synchronously in reply to a Set Feature Command.
 *
 * @return HAL_OK on a successful send, HAL_TIMEOUT if INT does not go low
 *         in time, or the HAL_StatusTypeDef of a failed SPI transfer (see
 *         bno085_send_packet()).
 */
HAL_StatusTypeDef bno085_set_feature(
	bno085_t *p,
	uint8_t report_id,
	uint8_t feature_flags,
	uint16_t change_sensitivity,
	uint32_t report_interval_us,
	uint32_t batch_interval_us,
	uint32_t sensor_specific_config
);

/**
 * bno085_read_rotation_vector
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Up to BNO085_GET_FEATURE_MAX_ATTEMPTS times: wakes the device (if needed)
 * and waits for INT to go low within BNO085_INT_TIMEOUT_MS, then reads a
 * packet via the same exact-length two-step read used by
 * bno085_get_feature() into cmd_buf/cmd_len. A packet matches if it was
 * received on BNO085_CHANNEL_INPUT_REPORTS, cmd_len >= 23, and its payload
 * is a Base Timestamp Reference (0xFB) immediately followed by a Rotation
 * Vector (0x05) report. Each non-matching packet is discarded and another
 * is read.
 *
 * On a match, the 14-byte Rotation Vector report is parsed into
 * rotation_vector: sequence number and status, the i/j/k/real quaternion
 * components as signed 16-bit Q14 values (with i_f/j_f/k_f/real_f as the
 * corresponding floats, value / 16384.0f), and the accuracy estimate as a
 * signed 16-bit Q12 value (with accuracy_rad = value / 4096.0f).
 *
 * @return HAL_OK on a matching report, HAL_TIMEOUT if INT does not go low
 *         in time on a read, HAL_ERROR if no matching packet is found
 *         within BNO085_GET_FEATURE_MAX_ATTEMPTS packets, or the
 *         HAL_StatusTypeDef of a failed SPI transfer.
 */
HAL_StatusTypeDef bno085_read_rotation_vector(bno085_t *p);

/**
 * bno085_print_rotation_vector
 * @param p - Pointer to a bno085_t struct populated by a successful call to
 *            bno085_read_rotation_vector()
 * @param huart - UART handle to print the parsed rotation vector to
 *
 * Transmits a single line over huart summarizing rotation_vector: the
 * quaternion components and accuracy as floats (i_f, j_f, k_f, real_f,
 * accuracy_rad), and the sequence and status fields.
 */
void bno085_print_rotation_vector(bno085_t *p, UART_HandleTypeDef *huart);

/**
 * bno085_send_command
 * @param p - Pointer to an initialized bno085_t struct
 * @param command - SH-2 command ID (1-127)
 * @param params - Pointer to a 9-byte array of command-specific parameters
 *                  (P0..P8)
 *
 * Builds a 12-byte SH-2 Command Request (0xF2) payload -
 * {0xF2, p->cmd_seq, command, params[0..9)} - and sends it on
 * BNO085_CHANNEL_CONTROL via bno085_send_packet(). On success,
 * p->last_cmd_seq is set to the sequence number that was sent and
 * p->cmd_seq is incremented (with uint8_t wraparound).
 *
 * @return HAL_OK on a successful send, HAL_TIMEOUT if INT does not go low
 *         in time, or the HAL_StatusTypeDef of a failed SPI transfer (see
 *         bno085_send_packet()). On any non-HAL_OK return, neither
 *         p->cmd_seq nor p->last_cmd_seq is modified.
 */
HAL_StatusTypeDef bno085_send_command(bno085_t *p, uint8_t command, const uint8_t params[9]);

/**
 * bno085_read_command_response
 * @param p - Pointer to an initialized bno085_t struct
 * @param command - SH-2 command ID that a prior bno085_send_command() call
 *                   was sent for
 *
 * Up to BNO085_GET_FEATURE_MAX_ATTEMPTS times: wakes the device (if needed)
 * and waits for INT to go low within BNO085_INT_TIMEOUT_MS, then reads a
 * packet via bno085_read_response() into cmd_buf/cmd_len. A packet matches
 * if cmd_len >= 20, cmd_buf[4] == BNO085_REPORT_ID_COMMAND_RESPONSE,
 * cmd_buf[6] == command, and cmd_buf[7] == p->last_cmd_seq. Each
 * non-matching packet is discarded and another is read. On a match,
 * cmd_buf[9..20) (R0..R10) are left populated for the caller to interpret.
 *
 * @return HAL_OK on a matching response, HAL_TIMEOUT if INT does not go low
 *         in time on a read, HAL_ERROR if no matching packet is found
 *         within BNO085_GET_FEATURE_MAX_ATTEMPTS packets, or the
 *         HAL_StatusTypeDef of a failed SPI transfer.
 */
HAL_StatusTypeDef bno085_read_command_response(bno085_t *p, uint8_t command);

/**
 * bno085_get_me_calibration
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Sends a Get ME Calibration command (BNO085_COMMAND_ME_CALIBRATION with
 * params {0, 0, 0, 0x01, 0, 0, 0, 0, 0}) via bno085_send_command(), then
 * calls bno085_read_command_response(p, BNO085_COMMAND_ME_CALIBRATION). On
 * HAL_OK, parses cmd_buf[9..15) into me_calibration: status (R0),
 * accel_enable, gyro_enable, mag_enable, planar_accel_enable, and
 * on_table_enable (R1..R5).
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of a failed send or
 *         response read. On any non-HAL_OK return, me_calibration is left
 *         unmodified.
 */
HAL_StatusTypeDef bno085_get_me_calibration(bno085_t *p);

/**
 * bno085_set_me_calibration
 * @param p - Pointer to an initialized bno085_t struct
 * @param accel_enable - 1 to enable accelerometer calibration, 0 to disable
 * @param gyro_enable - 1 to enable gyroscope calibration, 0 to disable
 * @param mag_enable - 1 to enable magnetometer calibration, 0 to disable
 * @param planar_accel_enable - 1 to enable planar accelerometer
 *                               calibration, 0 to disable
 * @param on_table_enable - 1 to enable on-table calibration, 0 to disable
 *
 * Sends a Configure ME Calibration command
 * (BNO085_COMMAND_ME_CALIBRATION with params {accel_enable, gyro_enable,
 * mag_enable, 0x00, planar_accel_enable, on_table_enable, 0, 0, 0}) via
 * bno085_send_command(), then calls
 * bno085_read_command_response(p, BNO085_COMMAND_ME_CALIBRATION). On
 * HAL_OK, parses cmd_buf[9..15) into me_calibration exactly as
 * bno085_get_me_calibration() does.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of a failed send or
 *         response read. On any non-HAL_OK return, me_calibration is left
 *         unmodified.
 */
HAL_StatusTypeDef bno085_set_me_calibration(
	bno085_t *p,
	uint8_t accel_enable,
	uint8_t gyro_enable,
	uint8_t mag_enable,
	uint8_t planar_accel_enable,
	uint8_t on_table_enable
);

/**
 * bno085_save_dcd
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Sends a Save DCD command (BNO085_COMMAND_SAVE_DCD with all-zero params)
 * via bno085_send_command(), then calls
 * bno085_read_command_response(p, BNO085_COMMAND_SAVE_DCD). On HAL_OK, sets
 * me_calibration.status from cmd_buf[9] (R0, 0 = success).
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of a failed send or
 *         response read.
 */
HAL_StatusTypeDef bno085_save_dcd(bno085_t *p);

#endif /* INC_BNO085_H_ */
