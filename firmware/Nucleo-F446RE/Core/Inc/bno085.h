
#ifndef INC_BNO085_H_
#define INC_BNO085_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Default 7-bit I2C address (SA0/ADR strapped low) */
#define BNO085_I2C_ADDRESS 0x4A

/* Size of the SHTP packet header (length, channel, sequence number) */
#define BNO085_SHTP_HEADER_SIZE 4

/* SHTP channels (fixed assignment per the SH-2 reference manual) */
#define BNO085_CHANNEL_COMMAND  0
#define BNO085_CHANNEL_CONTROL  2
#define BNO085_CHANNEL_REPORTS  3
#define BNO085_NUM_CHANNELS     6

/* SH-2 control channel report IDs */
#define BNO085_REPORT_PRODUCT_ID_REQUEST     0xF9
#define BNO085_REPORT_PRODUCT_ID_RESPONSE    0xF8
#define BNO085_REPORT_SET_FEATURE_COMMAND    0xFD
#define BNO085_SET_FEATURE_CMD_SIZE          17

#define BNO085_REPORT_COMMAND_REQUEST        0xF2
#define BNO085_COMMAND_ME_CALIBRATION        0x07
#define BNO085_ME_CALIBRATION_CONFIGURE      0x00
#define BNO085_COMMAND_REQUEST_SIZE          12

/* SH-2 sensor report (channel 3) report IDs and sizes, including the
 * 1-byte report ID */
#define BNO085_REPORT_BASE_TIMESTAMP_REFERENCE 0xFB
#define BNO085_BASE_TIMESTAMP_REFERENCE_SIZE   5

#define BNO085_REPORT_ROTATION_VECTOR          0x05
#define BNO085_ROTATION_VECTOR_SIZE            14

#define BNO085_REPORT_GAME_ROTATION_VECTOR     0x08
#define BNO085_GAME_ROTATION_VECTOR_SIZE       12

#define BNO085_REPORT_MAGNETIC_FIELD_CALIBRATED 0x03
#define BNO085_MAGNETIC_FIELD_CALIBRATED_SIZE   10

typedef struct {
	I2C_HandleTypeDef *port;
	uint16_t address;
	uint8_t seq[BNO085_NUM_CHANNELS];
} bno085_t;

/**
 * bno085_init
 * @param p - Pointer to bno085_t struct
 * @param in_port - Pointer to I2C_HandleTypeDef
 * @param in_address - The 7-bit I2C address
 *
 * Populates the handle - no I2C traffic is sent.
 */
void bno085_init(bno085_t *p, I2C_HandleTypeDef *in_port, uint16_t in_address);

/**
 * bno085_probe
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Checks the device acknowledges its I2C address.
 *
 * @return HAL_OK if the device responds, or the HAL_StatusTypeDef of the
 *         failed probe.
 */
HAL_StatusTypeDef bno085_probe(bno085_t *p);

/**
 * bno085_read_packet
 * @param p - Pointer to an initialized bno085_t struct
 * @param buf - Buffer to receive the SHTP packet (including its 4-byte
 *              header)
 * @param buf_size - Size of buf
 * @param out_len - Set to the packet length reported in the SHTP header
 *                  (may exceed buf_size; the packet is truncated to
 *                  buf_size in that case)
 *
 * Reads one SHTP packet via I2C: first the 4-byte header to determine the
 * packet length, then (if the packet is longer than the header) a second
 * read of the full packet up to buf_size.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the first failed
 *         I2C transfer.
 */
HAL_StatusTypeDef bno085_read_packet(bno085_t *p, uint8_t *buf, uint16_t buf_size, uint16_t *out_len);

/**
 * bno085_write_packet
 * @param p - Pointer to an initialized bno085_t struct
 * @param channel - SHTP channel number (e.g. BNO085_CHANNEL_CONTROL)
 * @param payload - Payload bytes to send (excluding the 4-byte SHTP header)
 * @param len - Length of payload
 *
 * Sends one SHTP packet via I2C, prefixing payload with a 4-byte header
 * (length, channel, sequence number). The per-channel sequence number is
 * tracked in p->seq and incremented after each write.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed I2C
 *         transfer. Returns HAL_ERROR if the packet would not fit in the
 *         internal write buffer.
 */
HAL_StatusTypeDef bno085_write_packet(bno085_t *p, uint8_t channel, const uint8_t *payload, uint16_t len);

/**
 * bno085_drain
 * @param p - Pointer to an initialized bno085_t struct
 *
 * Repeatedly reads and discards SHTP packets until the device reports
 * nothing left to deliver (a header-only packet, length <= 4). Per the
 * datasheet's SHTP I2C description, a packet longer than the read buffer is
 * delivered across multiple reads with the remaining length updated each
 * time; this drains any such backlog (e.g. the boot-time channel 0
 * advertisement) so a subsequent write/read pair gets a fresh response.
 */
void bno085_drain(bno085_t *p);

/**
 * bno085_set_feature
 * @param p - Pointer to an initialized bno085_t struct
 * @param report_id - The feature's report ID (e.g. BNO085_REPORT_ROTATION_VECTOR)
 * @param report_interval_us - Desired report interval in microseconds
 *
 * Sends a Set Feature Command (0xFD) on the SH-2 control channel to enable
 * a sensor report at the given interval, with all other fields (sensitivity,
 * batch interval, sensor-specific configuration) left at zero.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed I2C
 *         transfer.
 */
HAL_StatusTypeDef bno085_set_feature(bno085_t *p, uint8_t report_id, uint32_t report_interval_us);

/**
 * bno085_set_calibration
 * @param p - Pointer to an initialized bno085_t struct
 * @param accel - Enable continuous accelerometer calibration
 * @param gyro - Enable continuous gyroscope calibration
 * @param mag - Enable continuous magnetometer calibration
 *
 * Sends a Configure ME Calibration command (0xF2, command 0x07) on the SH-2
 * control channel, enabling or disabling the sensor hub's continuous
 * calibration routines for the accelerometer, gyro and magnetometer. Planar
 * accelerometer and on-table calibration are left disabled.
 *
 * @return HAL_OK on success, or the HAL_StatusTypeDef of the failed I2C
 *         transfer.
 */
HAL_StatusTypeDef bno085_set_calibration(bno085_t *p, bool accel, bool gyro, bool mag);

#endif /* INC_BNO085_H_ */
