

#include "bno085.h"
#include <string.h>

#define BNO085_I2C_TIMEOUT_MS 100
#define BNO085_WRITE_BUF_SIZE 32

void bno085_init(bno085_t *p, I2C_HandleTypeDef *in_port, uint16_t in_address)
{
	p->port = in_port;
	p->address = in_address;
	memset(p->seq, 0, sizeof(p->seq));
}

HAL_StatusTypeDef bno085_probe(bno085_t *p)
{
	return HAL_I2C_IsDeviceReady(p->port, (uint16_t)(p->address << 1), 3, BNO085_I2C_TIMEOUT_MS);
}

HAL_StatusTypeDef bno085_read_packet(bno085_t *p, uint8_t *buf, uint16_t buf_size, uint16_t *out_len)
{
	HAL_StatusTypeDef status;
	uint8_t header[BNO085_SHTP_HEADER_SIZE];

	status = HAL_I2C_Master_Receive(p->port, (uint16_t)(p->address << 1), header, sizeof(header), BNO085_I2C_TIMEOUT_MS);
	if (status != HAL_OK) {
		return status;
	}

	/* Length is little-endian, with bit 15 of byte 1 a continuation flag */
	uint16_t length = (uint16_t)(((header[1] & 0x7Fu) << 8) | header[0]);
	*out_len = length;

	if (length <= sizeof(header)) {
		memcpy(buf, header, (length < buf_size) ? length : buf_size);
		return HAL_OK;
	}

	uint16_t to_read = (length < buf_size) ? length : buf_size;
	return HAL_I2C_Master_Receive(p->port, (uint16_t)(p->address << 1), buf, to_read, BNO085_I2C_TIMEOUT_MS);
}

HAL_StatusTypeDef bno085_write_packet(bno085_t *p, uint8_t channel, const uint8_t *payload, uint16_t len)
{
	uint8_t buf[BNO085_WRITE_BUF_SIZE];
	uint16_t total_len = (uint16_t)(len + BNO085_SHTP_HEADER_SIZE);

	if (total_len > sizeof(buf)) {
		return HAL_ERROR;
	}

	buf[0] = (uint8_t)(total_len & 0xFF);
	buf[1] = (uint8_t)((total_len >> 8) & 0x7F);
	buf[2] = channel;
	buf[3] = p->seq[channel]++;
	memcpy(&buf[BNO085_SHTP_HEADER_SIZE], payload, len);

	return HAL_I2C_Master_Transmit(p->port, (uint16_t)(p->address << 1), buf, total_len, BNO085_I2C_TIMEOUT_MS);
}

void bno085_drain(bno085_t *p)
{
	uint8_t buf[32];
	uint16_t len;

	do {
		if (bno085_read_packet(p, buf, sizeof(buf), &len) != HAL_OK) {
			return;
		}
	} while (len > BNO085_SHTP_HEADER_SIZE);
}

HAL_StatusTypeDef bno085_set_feature(bno085_t *p, uint8_t report_id, uint32_t report_interval_us)
{
	uint8_t cmd[BNO085_SET_FEATURE_CMD_SIZE] = { 0 };

	cmd[0] = BNO085_REPORT_SET_FEATURE_COMMAND;
	cmd[1] = report_id;
	cmd[5] = (uint8_t)(report_interval_us & 0xFFu);
	cmd[6] = (uint8_t)((report_interval_us >> 8) & 0xFFu);
	cmd[7] = (uint8_t)((report_interval_us >> 16) & 0xFFu);
	cmd[8] = (uint8_t)((report_interval_us >> 24) & 0xFFu);

	return bno085_write_packet(p, BNO085_CHANNEL_CONTROL, cmd, sizeof(cmd));
}

HAL_StatusTypeDef bno085_set_calibration(bno085_t *p, bool accel, bool gyro, bool mag)
{
	uint8_t cmd[BNO085_COMMAND_REQUEST_SIZE] = { 0 };

	cmd[0] = BNO085_REPORT_COMMAND_REQUEST;
	cmd[2] = BNO085_COMMAND_ME_CALIBRATION;
	cmd[3] = accel ? 1 : 0;
	cmd[4] = gyro ? 1 : 0;
	cmd[5] = mag ? 1 : 0;
	cmd[6] = BNO085_ME_CALIBRATION_CONFIGURE;

	return bno085_write_packet(p, BNO085_CHANNEL_CONTROL, cmd, sizeof(cmd));
}
