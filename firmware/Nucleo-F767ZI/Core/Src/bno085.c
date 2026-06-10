#include "bno085.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#define BNO085_SPI_TIMEOUT_MS 100

/*
 * Drives RST low for BNO085_RESET_PULSE_MS then high, then polls INT for it
 * to go low (data ready) within BNO085_INT_TIMEOUT_MS, recording the
 * pre-reset INT level in int_initial and the time waited in int_wait_ms.
 *
 * Returns HAL_OK once INT is low, or HAL_TIMEOUT if it does not go low in
 * time.
 */
static HAL_StatusTypeDef bno085_reset_and_wait(bno085_t *p)
{
	p->int_initial = HAL_GPIO_ReadPin(p->int_port, p->int_pin);

	HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_RESET);
	HAL_Delay(BNO085_RESET_PULSE_MS);
	HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_SET);

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

	uint32_t start = HAL_GetTick();
	while (HAL_GPIO_ReadPin(p->int_port, p->int_pin) == GPIO_PIN_SET) {
		if ((HAL_GetTick() - start) >= BNO085_INT_TIMEOUT_MS) {
			p->int_wait_ms = HAL_GetTick() - start;
			return HAL_TIMEOUT;
		}
	}
	p->int_wait_ms = HAL_GetTick() - start;

	return HAL_OK;
}

void bno085_init(
	bno085_t *p,
	SPI_HandleTypeDef *in_port,
	GPIO_TypeDef *cs_port,
	uint16_t cs_pin,
	GPIO_TypeDef *rst_port,
	uint16_t rst_pin,
	GPIO_TypeDef *int_port,
	uint16_t int_pin
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
 * Polls INT for it to go low (the BNO08x asserts INT to indicate it is
 * ready for an SPI transaction, for both host reads and host writes) within
 * BNO085_INT_TIMEOUT_MS.
 *
 * Returns HAL_OK once INT is low, or HAL_TIMEOUT if it does not go low in
 * time.
 */
static HAL_StatusTypeDef bno085_wait_int_low(bno085_t *p)
{
	uint32_t start = HAL_GetTick();
	while (HAL_GPIO_ReadPin(p->int_port, p->int_pin) == GPIO_PIN_SET) {
		if ((HAL_GetTick() - start) >= BNO085_INT_TIMEOUT_MS) {
			return HAL_TIMEOUT;
		}
	}
	return HAL_OK;
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

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);

	/* First 4 bytes: send our header while simultaneously receiving
	 * whatever packet header the device has queued to send us. SPI is
	 * full-duplex, so if we don't capture these bytes here they are
	 * silently lost and the device's queue position is shifted. */
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(p->port, tx_buf, p->cmd_buf, 4, BNO085_SPI_TIMEOUT_MS);
	if (status != HAL_OK) {
		HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);
		return status;
	}

	uint16_t rx_length = (uint16_t)(p->cmd_buf[0] | (p->cmd_buf[1] << 8)) & 0x7FFF;
	uint16_t rx_total = (rx_length < BNO085_CMD_BUF_SIZE) ? rx_length : BNO085_CMD_BUF_SIZE;
	uint16_t transfer_total = (rx_total > tx_total) ? rx_total : tx_total;
	if (transfer_total > BNO085_CMD_BUF_SIZE) {
		transfer_total = BNO085_CMD_BUF_SIZE;
	}

	if (transfer_total > 4) {
		status = HAL_SPI_TransmitReceive(p->port, &tx_buf[4], &p->cmd_buf[4], transfer_total - 4, BNO085_SPI_TIMEOUT_MS);
	}

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	p->cmd_len = rx_total;
	p->tx_seq[channel]++;

	return HAL_OK;
}

/*
 * Waits for INT low, then performs the same exact-length two-step read as
 * bno085_read_advertisement() into cmd_buf, recording the received length in
 * cmd_len.
 */
static HAL_StatusTypeDef bno085_read_response(bno085_t *p)
{
	HAL_StatusTypeDef status = bno085_wait_int_low(p);
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

	/* The response to our request may not be at the head of the queue
	 * yet - other packets (e.g. the response to a previous Get Feature
	 * Request) can be read first, and bno085_send_packet() itself may
	 * already have captured a queued packet. Check the packet captured by
	 * each step and read another packet if it doesn't match, until a
	 * matching Get Feature Response is found or the retry limit is
	 * reached. */
	for (uint8_t attempt = 0; attempt < BNO085_GET_FEATURE_MAX_ATTEMPTS; attempt++) {
		if (attempt > 0) {
			status = bno085_read_response(p);
			if (status != HAL_OK) {
				return status;
			}
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
