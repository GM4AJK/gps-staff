#include "bno085.h"
#include <string.h>

#define BNO085_SPI_TIMEOUT_MS 100

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
	p->int_initial = HAL_GPIO_ReadPin(p->int_port, p->int_pin);

	HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_RESET);
	HAL_Delay(BNO085_RESET_PULSE_MS);
	HAL_GPIO_WritePin(p->rst_port, p->rst_pin, GPIO_PIN_SET);

	uint32_t start = HAL_GetTick();
	while (HAL_GPIO_ReadPin(p->int_port, p->int_pin) == GPIO_PIN_SET) {
		if ((HAL_GetTick() - start) >= BNO085_INT_TIMEOUT_MS) {
			p->int_wait_ms = HAL_GetTick() - start;
			return HAL_TIMEOUT;
		}
	}
	p->int_wait_ms = HAL_GetTick() - start;

	uint8_t tx_buf[BNO085_BRINGUP_BUF_SIZE] = { 0 };

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(p->port, tx_buf, p->rx_buf, BNO085_BRINGUP_BUF_SIZE, BNO085_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	p->shtp_length = (uint16_t)(p->rx_buf[0] | (p->rx_buf[1] << 8)) & 0x7FFF;
	p->shtp_channel = p->rx_buf[2];
	p->shtp_sequence = p->rx_buf[3];

	return HAL_OK;
}
