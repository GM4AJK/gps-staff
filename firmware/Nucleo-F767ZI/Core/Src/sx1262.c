

#include "sx1262.h"

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
