

#include "sx1262.h"

#include <stddef.h>

static HAL_StatusTypeDef sx1262_write(sx1262_t *p, const uint8_t *data, size_t len)
{
	HAL_StatusTypeDef status;

	status = sx1262_wait_busy(p);
	if (status != HAL_OK) {
		return status;
	}

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);
	status = HAL_SPI_Transmit(p->port, (uint8_t *)data, len, SX1262_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	return status;
}

void sx1262_init(
	sx1262_t *p,
	SPI_HandleTypeDef *in_port,
	GPIO_TypeDef *cs_port, uint16_t cs_pin,
	GPIO_TypeDef *reset_port, uint16_t reset_pin,
	GPIO_TypeDef *busy_port, uint16_t busy_pin)
{
	p->port = in_port;
	p->cs_port = cs_port;
	p->cs_pin = cs_pin;
	p->reset_port = reset_port;
	p->reset_pin = reset_pin;
	p->busy_port = busy_port;
	p->busy_pin = busy_pin;
}

HAL_StatusTypeDef sx1262_wait_busy(sx1262_t *p)
{
	uint32_t start = HAL_GetTick();

	while (HAL_GPIO_ReadPin(p->busy_port, p->busy_pin) == GPIO_PIN_SET) {
		if ((HAL_GetTick() - start) > SX1262_BUSY_TIMEOUT_MS) {
			return HAL_TIMEOUT;
		}
	}

	return HAL_OK;
}

HAL_StatusTypeDef sx1262_reset(sx1262_t *p)
{
	HAL_GPIO_WritePin(p->reset_port, p->reset_pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(p->reset_port, p->reset_pin, GPIO_PIN_SET);

	return sx1262_wait_busy(p);
}

HAL_StatusTypeDef sx1262_get_status(sx1262_t *p, uint8_t *out_status)
{
	uint8_t tx[2] = { SX1262_OP_GET_STATUS, SX1262_OP_NOP };
	uint8_t rx[2] = { 0 };
	HAL_StatusTypeDef status;

	status = sx1262_wait_busy(p);
	if (status != HAL_OK) {
		return status;
	}

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);
	status = HAL_SPI_TransmitReceive(p->port, tx, rx, sizeof(tx), SX1262_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	*out_status = rx[1];

	return HAL_OK;
}

HAL_StatusTypeDef sx1262_set_packet_type(sx1262_t *p, uint8_t packet_type)
{
	uint8_t tx[2] = { SX1262_OP_SET_PACKET_TYPE, packet_type };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_rf_frequency(sx1262_t *p, uint32_t freq_hz)
{
	uint32_t rf_freq = (uint32_t)(((uint64_t)freq_hz << 25) / SX1262_XTAL_HZ);
	uint8_t tx[5] = {
		SX1262_OP_SET_RF_FREQUENCY,
		(uint8_t)(rf_freq >> 24),
		(uint8_t)(rf_freq >> 16),
		(uint8_t)(rf_freq >> 8),
		(uint8_t)(rf_freq)
	};

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_modulation_params_lora(sx1262_t *p, uint8_t sf, uint8_t bw, uint8_t cr, uint8_t ldro)
{
	uint8_t tx[9] = { SX1262_OP_SET_MODULATION_PARAMS, sf, bw, cr, ldro, 0x00, 0x00, 0x00, 0x00 };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_packet_params_lora(sx1262_t *p, uint16_t preamble_len, uint8_t header_type, uint8_t payload_len, uint8_t crc_type, uint8_t invert_iq)
{
	uint8_t tx[7] = {
		SX1262_OP_SET_PACKET_PARAMS,
		(uint8_t)(preamble_len >> 8),
		(uint8_t)(preamble_len),
		header_type,
		payload_len,
		crc_type,
		invert_iq
	};

	return sx1262_write(p, tx, sizeof(tx));
}
