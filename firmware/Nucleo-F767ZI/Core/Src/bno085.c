#include "bno085.h"
#include "flags.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#define BNO085_SPI_TIMEOUT_MS 100

/*
 * Polls for INT to become active (the BNO08x asserts INT to indicate it is
 * ready for an SPI transaction, for both host reads and host writes) within
 * BNO085_INT_TIMEOUT_MS. INT is considered active as soon as either the
 * BNO085_INT edge flag is set (a falling edge latched by
 * HAL_GPIO_EXTI_Callback() at any point since it was last discarded by the
 * caller) or the INT GPIO pin currently reads low - this avoids missing a
 * brief assertion that occurs before this loop starts polling.
 *
 * Returns HAL_OK once INT is active, or HAL_TIMEOUT if it does not become
 * active in time.
 */
static HAL_StatusTypeDef bno085_wait_int_low(bno085_t *p)
{
	uint32_t start = HAL_GetTick();
	while (!flag_get_BNO085_INT() && HAL_GPIO_ReadPin(p->int_port, p->int_pin) == GPIO_PIN_SET) {
		if ((HAL_GetTick() - start) >= BNO085_INT_TIMEOUT_MS) {
			p->int_wait_ms = HAL_GetTick() - start;
			return HAL_TIMEOUT;
		}
	}
	p->int_wait_ms = HAL_GetTick() - start;
	return HAL_OK;
}

/*
 * Drives RST low for BNO085_RESET_PULSE_MS then high, then waits for INT to
 * become active (data ready) within BNO085_INT_TIMEOUT_MS via
 * bno085_wait_int_low(), recording the pre-reset INT level in int_initial
 * and the time waited in int_wait_ms.
 *
 * Returns HAL_OK once INT is active, or HAL_TIMEOUT if it does not become
 * active in time.
 */
static HAL_StatusTypeDef bno085_reset_and_wait(bno085_t *p)
{
	int loop = 0;

	p->int_initial = HAL_GPIO_ReadPin(p->int_port, p->int_pin);

	do {
		/* PS0/WAKE must be high from before reset until after the first
		 * assertion of INT, to select the SPI interface. */
		HAL_GPIO_WritePin(p->wake_port, p->wake_pin, GPIO_PIN_SET);

		HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_RESET);
		HAL_Delay(BNO085_RESET_PULSE_MS);
		HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_SET);

		/* Wait for 90ms * 2 after asserting a reset before testing the INT pin.
		 * During this delay period the INT pin is undefined.
		 * Then ensure the flag is reset */
		HAL_Delay(180);
	}
	while(loop++ < 1);



	/* Wait for INT to read high (deasserted) before polling for it to go
	 * low - rejects a spurious immediate-low reading while the chip is
	 * still in reset/boot. If it never reads high within the timeout,
	 * proceed anyway. */
	uint32_t deassert_start = HAL_GetTick();
	while (HAL_GPIO_ReadPin(p->int_port, p->int_pin) == GPIO_PIN_RESET) {
		if ((HAL_GetTick() - deassert_start) >= BNO085_INT_DEASSERT_TIMEOUT_MS) {
			break;
		}
	}

	/* Discard any latched INT edge from the RST pulse / in-reset period
	 * (the chip can read/drive INT low while still booting), so
	 * bno085_wait_int_low() below only observes the real post-boot
	 * data-ready assertion. */
	flag_get_BNO085_INT();

	return bno085_wait_int_low(p);
}

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
)
{
	memset(p, 0, sizeof(*p));

	p->port = in_port;
	p->cs_port = cs_port;
	p->cs_pin = cs_pin;
	p->rst_port = rst_port;
	p->rst_pin = rst_pin;
	p->int_port = int_port;
	p->int_pin = int_pin;
	p->wake_port = wake_port;
	p->wake_pin = wake_pin;
}

HAL_StatusTypeDef bno085_bringup(bno085_t *p)
{
	HAL_StatusTypeDef status = bno085_reset_and_wait(p);
	if (status != HAL_OK) {
		return status;
	}

	uint8_t tx_buf[BNO085_BRINGUP_BUF_SIZE] = { 0 };

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);
	status = HAL_SPI_TransmitReceive(p->port, tx_buf, p->rx_buf, BNO085_BRINGUP_BUF_SIZE, BNO085_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	p->shtp_length = (uint16_t)(p->rx_buf[0] | (p->rx_buf[1] << 8)) & 0x7FFF;
	p->shtp_channel = p->rx_buf[2];
	p->shtp_sequence = p->rx_buf[3];

	return HAL_OK;
}

HAL_StatusTypeDef bno085_read_advertisement(bno085_t *p)
{
	HAL_StatusTypeDef status = bno085_reset_and_wait(p);
	if (status != HAL_OK) {
		return status;
	}

	uint8_t tx_buf[BNO085_ADVERT_BUF_SIZE] = { 0 };

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);

	/* Read exactly the 4-byte SHTP header first, then exactly
	 * (length - 4) more payload bytes - reading past the packet's
	 * declared length would drain bytes from whatever the device queues
	 * next, leaving it misaligned for later reads. */
	status = HAL_SPI_TransmitReceive(p->port, tx_buf, p->advert_buf, 4, BNO085_SPI_TIMEOUT_MS);
	if (status != HAL_OK) {
		HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);
		return status;
	}

	uint16_t length = (uint16_t)(p->advert_buf[0] | (p->advert_buf[1] << 8)) & 0x7FFF;
	p->advert_len = (length < BNO085_ADVERT_BUF_SIZE) ? length : BNO085_ADVERT_BUF_SIZE;

	if (p->advert_len > 4) {
		status = HAL_SPI_TransmitReceive(p->port, &tx_buf[4], &p->advert_buf[4], p->advert_len - 4, BNO085_SPI_TIMEOUT_MS);
	}

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	return HAL_OK;
}

/*
 * If INT is already low, returns HAL_OK immediately. Otherwise discards any
 * stale latched INT edge, pulses PS0/WAKE low to ask the device to assert
 * INT, waits for INT to become active within BNO085_INT_TIMEOUT_MS (via
 * bno085_wait_int_low()), then returns PS0/WAKE high.
 */
static HAL_StatusTypeDef bno085_wake_and_wait_int_low(bno085_t *p)
{
	if (HAL_GPIO_ReadPin(p->int_port, p->int_pin) == GPIO_PIN_RESET) {
		p->int_wait_ms = 0;
		return HAL_OK;
	}

	/* Discard any stale latched INT edge before pulsing WAKE, so
	 * bno085_wait_int_low() below only observes an edge triggered by this
	 * pulse. */
	flag_get_BNO085_INT();

	HAL_GPIO_WritePin(p->wake_port, p->wake_pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = bno085_wait_int_low(p);
	HAL_GPIO_WritePin(p->wake_port, p->wake_pin, GPIO_PIN_SET);

	return status;
}

HAL_StatusTypeDef bno085_send_packet(bno085_t *p, uint8_t channel, const uint8_t *payload, uint16_t payload_len)
{
	uint8_t tx_buf[BNO085_CMD_BUF_SIZE] = { 0 };
	uint16_t tx_total = 4 + payload_len;

	tx_buf[0] = (uint8_t)(tx_total & 0xFF);
	tx_buf[1] = (uint8_t)(tx_total >> 8);
	tx_buf[2] = channel;
	tx_buf[3] = p->tx_seq[channel];
	memcpy(&tx_buf[4], payload, payload_len);

	HAL_StatusTypeDef status = bno085_wake_and_wait_int_low(p);
	if (status != HAL_OK) {
		return status;
	}

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);

	/* SPI is full-duplex: capture the bytes the device sends back during
	 * our write into cmd_buf rather than discarding them, so they're at
	 * least visible for debugging. The transfer is exactly tx_total bytes
	 * (our own packet length) - it is not extended to cover a longer
	 * pending packet the device may report, to avoid clocking extra
	 * zero-padded bytes into the device that aren't part of our packet. */
	status = HAL_SPI_TransmitReceive(p->port, tx_buf, p->cmd_buf, tx_total, BNO085_SPI_TIMEOUT_MS);

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	uint16_t rx_length = (uint16_t)(p->cmd_buf[0] | (p->cmd_buf[1] << 8)) & 0x7FFF;
	p->cmd_len = (rx_length < BNO085_CMD_BUF_SIZE) ? rx_length : BNO085_CMD_BUF_SIZE;
	p->tx_seq[channel]++;

	return HAL_OK;
}

/*
 * Wakes the device (if needed) and waits for INT low, then performs the same
 * exact-length two-step read as bno085_read_advertisement() into cmd_buf,
 * recording the received length in cmd_len.
 */
static HAL_StatusTypeDef bno085_read_response(bno085_t *p)
{
	HAL_StatusTypeDef status = bno085_wake_and_wait_int_low(p);
	if (status != HAL_OK) {
		return status;
	}

	uint8_t tx_buf[BNO085_CMD_BUF_SIZE] = { 0 };

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);

	status = HAL_SPI_TransmitReceive(p->port, tx_buf, p->cmd_buf, 4, BNO085_SPI_TIMEOUT_MS);
	if (status != HAL_OK) {
		HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);
		return status;
	}

	uint16_t length = (uint16_t)(p->cmd_buf[0] | (p->cmd_buf[1] << 8)) & 0x7FFF;
	uint16_t total_len = (length < BNO085_CMD_BUF_SIZE) ? length : BNO085_CMD_BUF_SIZE;

	if (total_len > 4) {
		status = HAL_SPI_TransmitReceive(p->port, &tx_buf[4], &p->cmd_buf[4], total_len - 4, BNO085_SPI_TIMEOUT_MS);
	}

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	p->cmd_len = total_len;

	return HAL_OK;
}

HAL_StatusTypeDef bno085_get_feature(bno085_t *p, uint8_t report_id)
{
	uint8_t payload[6] = { BNO085_REPORT_ID_GET_FEATURE_REQUEST, report_id, 0x00, 0x00, 0x00, 0x00 };

	HAL_StatusTypeDef status = bno085_send_packet(p, BNO085_CHANNEL_CONTROL, payload, sizeof(payload));
	if (status != HAL_OK) {
		return status;
	}

	/* The response to our request may not be the next packet read - a
	 * previously-queued packet (e.g. the response to an earlier Get
	 * Feature Request) can be read first. Read and discard packets until
	 * a matching Get Feature Response is found or the retry limit is
	 * reached. */
	for (uint8_t attempt = 0; attempt < BNO085_GET_FEATURE_MAX_ATTEMPTS; attempt++) {
		status = bno085_read_response(p);
		if (status != HAL_OK) {
			return status;
		}

		if (p->cmd_len >= 21 && p->cmd_buf[4] == BNO085_REPORT_ID_GET_FEATURE_RESPONSE && p->cmd_buf[5] == report_id) {
			p->feature.feature_report_id = report_id;
			p->feature.feature_flags = p->cmd_buf[6];
			p->feature.change_sensitivity = (uint16_t)(p->cmd_buf[7] | (p->cmd_buf[8] << 8));
			p->feature.report_interval_us = (uint32_t)(p->cmd_buf[9] | (p->cmd_buf[10] << 8) | (p->cmd_buf[11] << 16) | (p->cmd_buf[12] << 24));
			p->feature.batch_interval_us = (uint32_t)(p->cmd_buf[13] | (p->cmd_buf[14] << 8) | (p->cmd_buf[15] << 16) | (p->cmd_buf[16] << 24));
			p->feature.sensor_specific_config = (uint32_t)(p->cmd_buf[17] | (p->cmd_buf[18] << 8) | (p->cmd_buf[19] << 16) | (p->cmd_buf[20] << 24));

			return HAL_OK;
		}
	}

	return HAL_ERROR;
}

HAL_StatusTypeDef bno085_set_feature(
	bno085_t *p,
	uint8_t report_id,
	uint8_t feature_flags,
	uint16_t change_sensitivity,
	uint32_t report_interval_us,
	uint32_t batch_interval_us,
	uint32_t sensor_specific_config
)
{
	uint8_t payload[17] = {
		BNO085_REPORT_ID_SET_FEATURE_COMMAND,
		report_id,
		feature_flags,
		(uint8_t)(change_sensitivity & 0xFF),
		(uint8_t)(change_sensitivity >> 8),
		(uint8_t)(report_interval_us & 0xFF),
		(uint8_t)(report_interval_us >> 8),
		(uint8_t)(report_interval_us >> 16),
		(uint8_t)(report_interval_us >> 24),
		(uint8_t)(batch_interval_us & 0xFF),
		(uint8_t)(batch_interval_us >> 8),
		(uint8_t)(batch_interval_us >> 16),
		(uint8_t)(batch_interval_us >> 24),
		(uint8_t)(sensor_specific_config & 0xFF),
		(uint8_t)(sensor_specific_config >> 8),
		(uint8_t)(sensor_specific_config >> 16),
		(uint8_t)(sensor_specific_config >> 24),
	};

	return bno085_send_packet(p, BNO085_CHANNEL_CONTROL, payload, sizeof(payload));
}

HAL_StatusTypeDef bno085_read_rotation_vector(bno085_t *p)
{
	for (uint8_t attempt = 0; attempt < BNO085_GET_FEATURE_MAX_ATTEMPTS; attempt++) {
		HAL_StatusTypeDef status = bno085_read_response(p);
		if (status != HAL_OK) {
			return status;
		}

		if (p->cmd_buf[2] == BNO085_CHANNEL_INPUT_REPORTS && p->cmd_len >= 23
			&& p->cmd_buf[4] == BNO085_REPORT_ID_BASE_TIMESTAMP
			&& p->cmd_buf[9] == BNO085_REPORT_ID_ROTATION_VECTOR) {

			p->rotation_vector.sequence = p->cmd_buf[10];
			p->rotation_vector.status = p->cmd_buf[11] & 0x03;
			p->rotation_vector.i = (int16_t)(p->cmd_buf[13] | (p->cmd_buf[14] << 8));
			p->rotation_vector.j = (int16_t)(p->cmd_buf[15] | (p->cmd_buf[16] << 8));
			p->rotation_vector.k = (int16_t)(p->cmd_buf[17] | (p->cmd_buf[18] << 8));
			p->rotation_vector.real = (int16_t)(p->cmd_buf[19] | (p->cmd_buf[20] << 8));
			p->rotation_vector.accuracy = (int16_t)(p->cmd_buf[21] | (p->cmd_buf[22] << 8));

			p->rotation_vector.i_f = p->rotation_vector.i / (float)(1 << BNO085_ROTATION_VECTOR_Q_POINT);
			p->rotation_vector.j_f = p->rotation_vector.j / (float)(1 << BNO085_ROTATION_VECTOR_Q_POINT);
			p->rotation_vector.k_f = p->rotation_vector.k / (float)(1 << BNO085_ROTATION_VECTOR_Q_POINT);
			p->rotation_vector.real_f = p->rotation_vector.real / (float)(1 << BNO085_ROTATION_VECTOR_Q_POINT);
			p->rotation_vector.accuracy_rad = p->rotation_vector.accuracy / (float)(1 << BNO085_ROTATION_VECTOR_ACCURACY_Q_POINT);

			return HAL_OK;
		}
	}

	return HAL_ERROR;
}

/*
 * Formats a signed Q-point fixed-point value as a fixed-4-decimal-place
 * string (e.g. "-0.6831"), without relying on printf's %f - newlib-nano
 * (the default STM32 C library) omits float formatting support unless the
 * linker is told to pull it in.
 */
static int bno085_format_q(int16_t raw, int q_point, char *buf, size_t bufsize)
{
	int32_t scaled = ((int32_t)raw * 10000) / (1 << q_point);
	int32_t int_part = scaled / 10000;
	int32_t frac_part = scaled % 10000;

	if (frac_part < 0) {
		frac_part = -frac_part;
	}

	if (int_part == 0 && scaled < 0) {
		return snprintf(buf, bufsize, "-0.%04ld", (long)frac_part);
	}

	return snprintf(buf, bufsize, "%ld.%04ld", (long)int_part, (long)frac_part);
}

void bno085_print_rotation_vector(bno085_t *p, UART_HandleTypeDef *huart)
{
	char buf[64];
	int len;

	len = snprintf(buf, sizeof(buf), "rv: i=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->rotation_vector.i, BNO085_ROTATION_VECTOR_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " j=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->rotation_vector.j, BNO085_ROTATION_VECTOR_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " k=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->rotation_vector.k, BNO085_ROTATION_VECTOR_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " real=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->rotation_vector.real, BNO085_ROTATION_VECTOR_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " acc=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->rotation_vector.accuracy, BNO085_ROTATION_VECTOR_ACCURACY_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " seq=%u status=%u\r\n", p->rotation_vector.sequence, p->rotation_vector.status);
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
}

HAL_StatusTypeDef bno085_read_magnetic_field(bno085_t *p)
{
	for (uint8_t attempt = 0; attempt < BNO085_GET_FEATURE_MAX_ATTEMPTS; attempt++) {
		HAL_StatusTypeDef status = bno085_read_response(p);
		if (status != HAL_OK) {
			return status;
		}

		if (p->cmd_buf[2] == BNO085_CHANNEL_INPUT_REPORTS && p->cmd_len >= 19
			&& p->cmd_buf[4] == BNO085_REPORT_ID_BASE_TIMESTAMP
			&& p->cmd_buf[9] == BNO085_REPORT_ID_MAGNETIC_FIELD) {

			p->magnetic_field.sequence = p->cmd_buf[10];
			p->magnetic_field.status = p->cmd_buf[11] & 0x03;
			p->magnetic_field.x = (int16_t)(p->cmd_buf[13] | (p->cmd_buf[14] << 8));
			p->magnetic_field.y = (int16_t)(p->cmd_buf[15] | (p->cmd_buf[16] << 8));
			p->magnetic_field.z = (int16_t)(p->cmd_buf[17] | (p->cmd_buf[18] << 8));

			p->magnetic_field.x_f = p->magnetic_field.x / (float)(1 << BNO085_MAGNETIC_FIELD_Q_POINT);
			p->magnetic_field.y_f = p->magnetic_field.y / (float)(1 << BNO085_MAGNETIC_FIELD_Q_POINT);
			p->magnetic_field.z_f = p->magnetic_field.z / (float)(1 << BNO085_MAGNETIC_FIELD_Q_POINT);

			return HAL_OK;
		}
	}

	return HAL_ERROR;
}

void bno085_print_magnetic_field(bno085_t *p, UART_HandleTypeDef *huart)
{
	char buf[64];
	int len;

	len = snprintf(buf, sizeof(buf), "mag: x=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->magnetic_field.x, BNO085_MAGNETIC_FIELD_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " y=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->magnetic_field.y, BNO085_MAGNETIC_FIELD_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " z=");
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
	len = bno085_format_q(p->magnetic_field.z, BNO085_MAGNETIC_FIELD_Q_POINT, buf, sizeof(buf));
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

	len = snprintf(buf, sizeof(buf), " seq=%u status=%u\r\n", p->magnetic_field.sequence, p->magnetic_field.status);
	HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
}

HAL_StatusTypeDef bno085_send_command(bno085_t *p, uint8_t command, const uint8_t params[9])
{
	uint8_t payload[12];

	payload[0] = BNO085_REPORT_ID_COMMAND_REQUEST;
	payload[1] = p->cmd_seq;
	payload[2] = command;
	memcpy(&payload[3], params, 9);

	HAL_StatusTypeDef status = bno085_send_packet(p, BNO085_CHANNEL_CONTROL, payload, sizeof(payload));
	if (status != HAL_OK) {
		return status;
	}

	p->last_cmd_seq = p->cmd_seq;
	p->cmd_seq++;

	return HAL_OK;
}

HAL_StatusTypeDef bno085_read_command_response(bno085_t *p, uint8_t command)
{
	uint8_t attempt = 0;

	/* Empty (cmd_len == 0) packets happen when bno085_wake_and_wait_int_low()
	 * sees a stale latched INT edge flag (left over from a streamed
	 * Rotation Vector / Magnetic Field report) and proceeds to read before
	 * the device actually has new data. These don't count against the
	 * attempt budget - only packets that actually carry data do. */
	for (uint8_t reads = 0; reads < BNO085_COMMAND_RESPONSE_MAX_TOTAL_READS; reads++) {
		HAL_StatusTypeDef status = bno085_read_response(p);
		if (status != HAL_OK) {
			return status;
		}

		if (p->cmd_len == 0) {
			/* Give the device a moment to actually queue its response
			 * before hammering it with another back-to-back SPI
			 * transaction. */
			HAL_Delay(1);
			continue;
		}

		if (p->cmd_len >= 20 && p->cmd_buf[4] == BNO085_REPORT_ID_COMMAND_RESPONSE
			&& p->cmd_buf[6] == command && p->cmd_buf[7] == p->last_cmd_seq) {
			return HAL_OK;
		}

		if (++attempt >= BNO085_COMMAND_RESPONSE_MAX_ATTEMPTS) {
			break;
		}
	}

	return HAL_ERROR;
}

static void bno085_parse_me_calibration_response(bno085_t *p)
{
	p->me_calibration.status = p->cmd_buf[9];
	p->me_calibration.accel_enable = p->cmd_buf[10];
	p->me_calibration.gyro_enable = p->cmd_buf[11];
	p->me_calibration.mag_enable = p->cmd_buf[12];
	p->me_calibration.planar_accel_enable = p->cmd_buf[13];
	p->me_calibration.on_table_enable = p->cmd_buf[14];
}

HAL_StatusTypeDef bno085_get_me_calibration(bno085_t *p)
{
	const uint8_t params[9] = { 0, 0, 0, 0x01, 0, 0, 0, 0, 0 };

	HAL_StatusTypeDef status = bno085_send_command(p, BNO085_COMMAND_ME_CALIBRATION, params);
	if (status != HAL_OK) {
		return status;
	}

	status = bno085_read_command_response(p, BNO085_COMMAND_ME_CALIBRATION);
	if (status != HAL_OK) {
		return status;
	}

	bno085_parse_me_calibration_response(p);

	return HAL_OK;
}

HAL_StatusTypeDef bno085_set_me_calibration(
	bno085_t *p,
	uint8_t accel_enable,
	uint8_t gyro_enable,
	uint8_t mag_enable,
	uint8_t planar_accel_enable,
	uint8_t on_table_enable
)
{
	const uint8_t params[9] = { accel_enable, gyro_enable, mag_enable, 0x00, planar_accel_enable, on_table_enable, 0, 0, 0 };

	HAL_StatusTypeDef status = bno085_send_command(p, BNO085_COMMAND_ME_CALIBRATION, params);
	if (status != HAL_OK) {
		return status;
	}

	status = bno085_read_command_response(p, BNO085_COMMAND_ME_CALIBRATION);
	if (status != HAL_OK) {
		return status;
	}

	bno085_parse_me_calibration_response(p);

	return HAL_OK;
}

HAL_StatusTypeDef bno085_save_dcd(bno085_t *p)
{
	const uint8_t params[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	HAL_StatusTypeDef status = bno085_send_command(p, BNO085_COMMAND_SAVE_DCD, params);
	if (status != HAL_OK) {
		return status;
	}

	status = bno085_read_command_response(p, BNO085_COMMAND_SAVE_DCD);
	if (status != HAL_OK) {
		return status;
	}

	p->me_calibration.status = p->cmd_buf[9];

	return HAL_OK;
}

HAL_StatusTypeDef bno085_set_periodic_dcd_save(bno085_t *p, uint8_t enable)
{
	const uint8_t params[9] = { enable ? 0x00 : 0x01, 0, 0, 0, 0, 0, 0, 0, 0 };

	return bno085_send_command(p, BNO085_COMMAND_DCD_PERIODIC_SAVE, params);
}

HAL_StatusTypeDef bno085_start_calibration(bno085_t *p, uint32_t interval_us)
{
	const uint8_t params[9] = {
		BNO085_SIMPLE_CALIBRATION_START,
		(uint8_t)(interval_us & 0xFF),
		(uint8_t)((interval_us >> 8) & 0xFF),
		(uint8_t)((interval_us >> 16) & 0xFF),
		(uint8_t)((interval_us >> 24) & 0xFF),
		0, 0, 0, 0
	};

	HAL_StatusTypeDef status = bno085_send_command(p, BNO085_COMMAND_SIMPLE_CALIBRATION, params);
	if (status != HAL_OK) {
		return status;
	}

	status = bno085_read_command_response(p, BNO085_COMMAND_SIMPLE_CALIBRATION);
	if (status != HAL_OK) {
		return status;
	}

	p->simple_calibration.start_status = p->cmd_buf[9];

	return HAL_OK;
}

HAL_StatusTypeDef bno085_finish_calibration(bno085_t *p)
{
	const uint8_t params[9] = { BNO085_SIMPLE_CALIBRATION_FINISH, 0, 0, 0, 0, 0, 0, 0, 0 };

	HAL_StatusTypeDef status = bno085_send_command(p, BNO085_COMMAND_SIMPLE_CALIBRATION, params);
	if (status != HAL_OK) {
		return status;
	}

	status = bno085_read_command_response(p, BNO085_COMMAND_SIMPLE_CALIBRATION);
	if (status != HAL_OK) {
		return status;
	}

	p->simple_calibration.finish_status = p->cmd_buf[10];

	return HAL_OK;
}

void bno085_print_advertisement(bno085_t *p, UART_HandleTypeDef *huart)
{
	char buf[128];
	int len;
	uint16_t offset = 4;

	while (offset + 2 <= p->advert_len && offset + 2 <= BNO085_ADVERT_BUF_SIZE) {
		uint8_t tag = p->advert_buf[offset];
		uint8_t value_len = p->advert_buf[offset + 1];

		if (offset + 2 + value_len > p->advert_len || offset + 2 + value_len > BNO085_ADVERT_BUF_SIZE) {
			break;
		}

		const uint8_t *value = &p->advert_buf[offset + 2];

		if (value_len == 0) {
			len = snprintf(buf, sizeof(buf), "tag %u: (empty)\r\n", tag);
		} else {
			/* Trim a single trailing NUL terminator before checking for a
			 * printable string, so C-string fields (e.g. channel/app
			 * names) print as quoted strings rather than hex. */
			uint8_t str_len = value_len;
			if (value[str_len - 1] == '\0') {
				str_len--;
			}

			bool printable = (str_len > 0);
			for (uint8_t i = 0; i < str_len; i++) {
				if (!isprint(value[i])) {
					printable = false;
					break;
				}
			}

			if (printable) {
				len = snprintf(buf, sizeof(buf), "tag %u: \"%.*s\"\r\n", tag, (int)str_len, (const char *)value);
			} else if (value_len == 1) {
				len = snprintf(buf, sizeof(buf), "tag %u: %u\r\n", tag, value[0]);
			} else {
				len = snprintf(buf, sizeof(buf), "tag %u:", tag);
				HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
				for (uint8_t i = 0; i < value_len; i++) {
					len = snprintf(buf, sizeof(buf), " %02X", value[i]);
					HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);
				}
				len = snprintf(buf, sizeof(buf), "\r\n");
			}
		}

		HAL_UART_Transmit(huart, (uint8_t *)buf, len, 100);

		offset += 2 + value_len;
	}
}
