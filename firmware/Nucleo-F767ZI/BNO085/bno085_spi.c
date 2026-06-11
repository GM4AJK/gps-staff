/*
 * bno085_spi.c
 *
 *  Created on: 11 Jun 2026
 *      Author: kirkh
 *
 *  Based on Dave Wheeler's sh2_spi_hal driver
 *  but modified to allow for multiple devices
 *  on different SPI buses.
 */

#if 0

#include <string.h>

#include "bno085_spi.h"

#define READ_LEN (4)

#define ASSERT_CS(p) do { HAL_GPIO_WritePin(p->chip_select.port, p->chip_select.pin, GPIO_PIN_RESET); } while(0);
#define RELEASE_CS(p) do { HAL_GPIO_WritePin(p->chip_select.port, p->chip_select.pin, GPIO_PIN_SET); } while(0);
#define ASSERT_WAKE(p) do { HAL_GPIO_WritePin(p->ps0_wake.port, p->ps0_wake.pin, GPIO_PIN_RESET); } while(0);
#define RELEASE_WAKE(p) do { HAL_GPIO_WritePin(p->ps0_wake.port, p->ps0_wake.pin, GPIO_PIN_SET); } while(0);
#define ASSERT_RESET(p) do { HAL_GPIO_WritePin(p->reset.port, p->reset.pin, GPIO_PIN_RESET); } while(0);
#define RELEASE_RESET(p) do { HAL_GPIO_WritePin(p->reset.port, p->reset.pin, GPIO_PIN_SET); } while(0);
#define READ_INT_SIG(p) HAL_GPIO_ReadPin(p->interrupt.port, p->interrupt.pin)

enum Registers
{
	CHANNEL_COMMAND = 0,
	CHANNEL_EXECUTABLE = 1,
	CHANNEL_CONTROL = 2,
	CHANNEL_REPORTS = 3,
	CHANNEL_WAKE_REPORTS = 4,
	CHANNEL_GYRO = 5
};

#define SHTP_REPORT_COMMAND_RESPONSE 0xF1
#define SHTP_REPORT_COMMAND_REQUEST 0xF2
#define SHTP_REPORT_FRS_READ_RESPONSE 0xF3
#define SHTP_REPORT_FRS_READ_REQUEST 0xF4
#define SHTP_REPORT_PRODUCT_ID_RESPONSE 0xF8
#define SHTP_REPORT_PRODUCT_ID_REQUEST 0xF9
#define SHTP_REPORT_BASE_TIMESTAMP 0xFB
#define SHTP_REPORT_SET_FEATURE_COMMAND 0xFD

#define SENSOR_REPORTID_ACCELEROMETER 0x01
#define SENSOR_REPORTID_GYROSCOPE 0x02
#define SENSOR_REPORTID_MAGNETIC_FIELD 0x03
#define SENSOR_REPORTID_LINEAR_ACCELERATION 0x04
#define SENSOR_REPORTID_ROTATION_VECTOR 0x05
#define SENSOR_REPORTID_GRAVITY 0x06
#define SENSOR_REPORTID_GAME_ROTATION_VECTOR 0x08
#define SENSOR_REPORTID_GEOMAGNETIC_ROTATION_VECTOR 0x09
#define SENSOR_REPORTID_TAP_DETECTOR 0x10
#define SENSOR_REPORTID_STEP_COUNTER 0x11
#define SENSOR_REPORTID_STABILITY_CLASSIFIER 0x13
#define SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER 0x1E

static void dummy_spi_op(bno085_t *p)
{
	uint8_t dummyTx[1];
	uint8_t dummyRx[1];
	memset(dummyTx, 0xAA, sizeof(dummyTx));
	HAL_SPI_TransmitReceive(p->spi, dummyTx, dummyRx, sizeof(dummyTx), 2);
}

static bool wait_for_spi(bno085_t *p)
{
	uint32_t to = HAL_GetTick();
	while(HAL_GPIO_ReadPin(p->interrupt.port, p->interrupt.pin) == GPIO_PIN_SET) {
		if(HAL_GetTick() > to) return false;
	}
	return true;
}

static void spi_read_write(bno085_t *p, uint8_t *txdata, uint8_t *rxdata, uint16_t sz)
{
	ASSERT_CS(p);
	HAL_SPI_TransmitReceive(p->spi, txdata, rxdata, sz, 5000);
	RELEASE_CS(p);
}

static bool receive_packet(bno085_t *p)
{
	if(!wait_for_spi(p)) {
		return false;
	}
	// Read header (4 bytes)
	uint8_t txBuf[4] = {0xFF, 0xFF, 0xFF, 0xFF};
	spi_read_write(p, txBuf, p->shtp_header, sizeof(p->shtp_header));
	// Calculate packet length
	uint16_t packetLSB = p->shtp_header[0];
	uint16_t packetMSB = p->shtp_header[1];
	uint16_t packetLength = ((uint16_t)packetMSB << 8) | packetLSB;
	if(packetLength == 0 || packetLength == 0xFFFF) return false;
	if(packetLength > 512) packetLength = 512;
	packetLength &= ~(1 << 15); // Clear continuation bit
	// Read data
	uint8_t paddingLength = 0;
	uint16_t dataLength = packetLength - 4;
	if(dataLength > 0) {
		if(dataLength % 4 != 0) {
			paddingLength = 4 - (dataLength % 4);
		}
		uint8_t txData[dataLength + paddingLength];
		memset(txData, 0xFF, dataLength + paddingLength);
		spi_read_write(p, txBuf, p->shtp_data, dataLength + paddingLength);
	}
	uint8_t channel_num = p->shtp_header[2];
	//uint8_t seq_num = p->shtp_header[3];
	if(channel_num == CHANNEL_REPORTS || channel_num == CHANNEL_WAKE_REPORTS) {
		//parseInputReport();
		p->data_available = true;
	}
	else if(channel_num == CHANNEL_CONTROL) {
		//parseCommandReport();
	}
	return true;
}

static bool send_packet(bno085_t *p, uint8_t channel_num, uint8_t data_len)
{
	uint16_t total_len = data_len + 4;
	uint8_t packet[total_len];
	packet[0] = total_len & 0xFF;
	packet[1] = total_len >> 8;
	packet[2] = channel_num;
	packet[3] = p->seq_number[channel_num]++;
	memcpy(&packet[4], p->shtp_data, data_len);
	if(!wait_for_spi(p)) return false;
	uint8_t rxbuf[total_len];
	spi_read_write(p, packet, rxbuf, total_len);
	return true;
}

static void soft_reset(bno085_t *p)
{
	p->shtp_data[0] = 1;
	send_packet(p, CHANNEL_EXECUTABLE, 1);
}

HAL_StatusTypeDef bno085_init(bno085_t *p)
{
	for(int i = 0; i < 6; i++) p->seq_number[i] = 0;

	// Initialise device (BNO085-Datasheet.pdf section 6.5.3)
	p->spiState = SPI_INIT;
	p->spiState = SPI_DUMMY;
	RELEASE_CS(p);
	ASSERT_RESET(p);
	dummy_spi_op(p);
	p->us_delay_fp(10);
	RELEASE_RESET(p);
	p->us_delay_fp(9000);
	uint32_t timeout = HAL_GetTick() + 3000;
	while(READ_INT_SIG(p) == GPIO_PIN_SET) {
		if(HAL_GetTick() > timeout) return HAL_ERROR;
	}
	p->spiState = SPI_IDLE;
	// Interrupt pin now  low indicating it has an advertisiment message
	receive_packet(p);
	// Soft reset to put sensor into a known state
	soft_reset(p);
	HAL_Delay(250);
	timeout = HAL_GetTick() + 3000;
	while(READ_INT_SIG(p) == GPIO_PIN_SET) {
		if(HAL_GetTick() > timeout) return HAL_ERROR;
	}
	// advertisiment message after soft reset
	receive_packet(p);
	return HAL_OK;
}

static bool set_feature_command(bno085_t *p,
		uint8_t report_id, uint16_t t_between,
		uint32_t config)
{
	memset(p->shtp_data, 0, sizeof(p->shtp_data));
	p->shtp_data[0] = SHTP_REPORT_SET_FEATURE_COMMAND;
	p->shtp_data[1] = report_id;
	p->shtp_data[4] = (t_between >> 0) & 0xFF;
	p->shtp_data[5] = (t_between >> 8) & 0xFF;
	p->shtp_data[12] = (config >> 0) & 0xFF;
	p->shtp_data[13] = (config >> 8) & 0xFF;
	p->shtp_data[14] = (config >> 16) & 0xFF;
	p->shtp_data[15] = (config >> 24) & 0xFF;
	return send_packet(p, CHANNEL_CONTROL, 17);
}

bool bno085_enable_rotation_vector(bno085_t *p, uint16_t t_between)
{
	return set_feature_command(p, SENSOR_REPORTID_ROTATION_VECTOR, t_between, 0);
}

bool bno085_enableGameRotationVector(bno085_t *p, uint16_t t_between)
{
	return set_feature_command(p, SENSOR_REPORTID_GAME_ROTATION_VECTOR, t_between, 0);
}

bool bno085_enableAccelerometer(bno085_t *p, uint16_t t_between)
{
	return set_feature_command(p, SENSOR_REPORTID_ACCELEROMETER, t_between, 0);
}

bool bno085_enableGyro(bno085_t *p, uint16_t t_between)
{
	return set_feature_command(p, SENSOR_REPORTID_GYROSCOPE, t_between, 0);
}

bool bno085_enableMagnetometer(bno085_t *p, uint16_t t_between)
{
	return set_feature_command(p, SENSOR_REPORTID_MAGNETIC_FIELD, t_between, 0);
}

bool bno085_enableLinearAccelerometer(bno085_t *p, uint16_t t_between)
{
	return set_feature_command(p, SENSOR_REPORTID_LINEAR_ACCELERATION, t_between, 0);
}

bool bno085_enableGravity(bno085_t *p, uint16_t t_between)
{
	return set_feature_command(p, SENSOR_REPORTID_GRAVITY, t_between, 0);
}

#endif
