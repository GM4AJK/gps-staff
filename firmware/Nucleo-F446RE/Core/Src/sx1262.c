

#include "sx1262.h"

#include <stddef.h>
#include <string.h>

#ifdef SX1262_WITH_LOGGING
#include <stdio.h>
#include <stdarg.h>

static void sx1262_log(sx1262_t *p, const char *fmt, ...)
{
	char buf[128];
	va_list args;
	int len;

	if (p->logger == NULL) {
		return;
	}

	va_start(args, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	p->logger(buf, len);
}

#define SX1262_LOG(p, ...) sx1262_log((p), __VA_ARGS__)
#else
#define SX1262_LOG(p, ...) ((void)0)
#endif /* SX1262_WITH_LOGGING */

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
	p->rx_done = NULL;
	p->tx_done = NULL;
	p->rx_timeout = NULL;
	p->tx_timeout = NULL;
#ifdef SX1262_WITH_LOGGING
	p->logger = NULL;
#endif
}

void sx1262_set_rx_done_callback(sx1262_t *p, void (*callback)(sx1262_t *p, const uint8_t *payload, size_t len, int8_t rssi, int8_t snr_quarter_db))
{
	p->rx_done = callback;
}

void sx1262_set_tx_done_callback(sx1262_t *p, void (*callback)(sx1262_t *p))
{
	p->tx_done = callback;
}

void sx1262_set_rx_timeout_callback(sx1262_t *p, void (*callback)(sx1262_t *p))
{
	p->rx_timeout = callback;
}

void sx1262_set_tx_timeout_callback(sx1262_t *p, void (*callback)(sx1262_t *p))
{
	p->tx_timeout = callback;
}

#ifdef SX1262_WITH_LOGGING
void sx1262_set_logger_callback(sx1262_t *p, void (*logger)(const char *buf, int len))
{
	p->logger = logger;
}
#endif

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

HAL_StatusTypeDef sx1262_calibrate_image(sx1262_t *p, uint8_t freq1, uint8_t freq2)
{
	uint8_t tx[3] = { SX1262_OP_CALIBRATE_IMAGE, freq1, freq2 };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_dio3_as_tcxo_ctrl(sx1262_t *p, uint8_t tcxo_voltage, uint32_t delay)
{
	uint8_t tx[5] = { SX1262_OP_SET_DIO3_AS_TCXO_CTRL, tcxo_voltage, (uint8_t)(delay >> 16), (uint8_t)(delay >> 8), (uint8_t)(delay) };

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

HAL_StatusTypeDef sx1262_set_pa_config(sx1262_t *p, uint8_t pa_duty_cycle, uint8_t hp_max, uint8_t device_sel)
{
	uint8_t tx[5] = { SX1262_OP_SET_PA_CONFIG, pa_duty_cycle, hp_max, device_sel, 0x01 };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_tx_params(sx1262_t *p, int8_t power, uint8_t ramp_time)
{
	uint8_t tx[3] = { SX1262_OP_SET_TX_PARAMS, (uint8_t)power, ramp_time };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_buffer_base_address(sx1262_t *p, uint8_t tx_base_addr, uint8_t rx_base_addr)
{
	uint8_t tx[3] = { SX1262_OP_SET_BUFFER_BASE_ADDRESS, tx_base_addr, rx_base_addr };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_write_buffer(sx1262_t *p, uint8_t offset, const uint8_t *data, size_t len)
{
	uint8_t tx[2 + SX1262_MAX_PAYLOAD_LEN];

	if (len > SX1262_MAX_PAYLOAD_LEN) {
		return HAL_ERROR;
	}

	tx[0] = SX1262_OP_WRITE_BUFFER;
	tx[1] = offset;
	memcpy(&tx[2], data, len);

	return sx1262_write(p, tx, 2 + len);
}

HAL_StatusTypeDef sx1262_read_buffer(sx1262_t *p, uint8_t offset, uint8_t *out_data, size_t len)
{
	uint8_t tx[3 + SX1262_MAX_PAYLOAD_LEN] = { 0 };
	uint8_t rx[3 + SX1262_MAX_PAYLOAD_LEN] = { 0 };
	HAL_StatusTypeDef status;

	if (len > SX1262_MAX_PAYLOAD_LEN) {
		return HAL_ERROR;
	}

	tx[0] = SX1262_OP_READ_BUFFER;
	tx[1] = offset;

	status = sx1262_wait_busy(p);
	if (status != HAL_OK) {
		return status;
	}

	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_RESET);
	status = HAL_SPI_TransmitReceive(p->port, tx, rx, 3 + len, SX1262_SPI_TIMEOUT_MS);
	HAL_GPIO_WritePin(p->cs_port, p->cs_pin, GPIO_PIN_SET);

	if (status != HAL_OK) {
		return status;
	}

	memcpy(out_data, &rx[3], len);

	return HAL_OK;
}

HAL_StatusTypeDef sx1262_set_tx(sx1262_t *p, uint32_t timeout)
{
	uint8_t tx[4] = { SX1262_OP_SET_TX, (uint8_t)(timeout >> 16), (uint8_t)(timeout >> 8), (uint8_t)(timeout) };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_rx(sx1262_t *p, uint32_t timeout)
{
	uint8_t tx[4] = { SX1262_OP_SET_RX, (uint8_t)(timeout >> 16), (uint8_t)(timeout >> 8), (uint8_t)(timeout) };

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_set_dio_irq_params(sx1262_t *p, uint16_t irq_mask, uint16_t dio1_mask, uint16_t dio2_mask, uint16_t dio3_mask)
{
	uint8_t tx[9] = {
		SX1262_OP_SET_DIO_IRQ_PARAMS,
		(uint8_t)(irq_mask >> 8), (uint8_t)(irq_mask),
		(uint8_t)(dio1_mask >> 8), (uint8_t)(dio1_mask),
		(uint8_t)(dio2_mask >> 8), (uint8_t)(dio2_mask),
		(uint8_t)(dio3_mask >> 8), (uint8_t)(dio3_mask)
	};

	return sx1262_write(p, tx, sizeof(tx));
}

HAL_StatusTypeDef sx1262_get_irq_status(sx1262_t *p, uint16_t *out_irq)
{
	uint8_t tx[4] = { SX1262_OP_GET_IRQ_STATUS, SX1262_OP_NOP, SX1262_OP_NOP, SX1262_OP_NOP };
	uint8_t rx[4] = { 0 };
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

	*out_irq = ((uint16_t)rx[2] << 8) | rx[3];

	return HAL_OK;
}

HAL_StatusTypeDef sx1262_get_packet_status(sx1262_t *p, int8_t *out_rssi_pkt, int8_t *out_snr_pkt_quarter_db)
{
	uint8_t tx[5] = { SX1262_OP_GET_PACKET_STATUS, SX1262_OP_NOP, SX1262_OP_NOP, SX1262_OP_NOP, SX1262_OP_NOP };
	uint8_t rx[5] = { 0 };
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

	*out_rssi_pkt = (int8_t)(-rx[2] / 2);
	*out_snr_pkt_quarter_db = (int8_t)rx[3];

	return HAL_OK;
}

HAL_StatusTypeDef sx1262_clear_irq_status(sx1262_t *p, uint16_t clear_mask)
{
	uint8_t tx[3] = { SX1262_OP_CLEAR_IRQ_STATUS, (uint8_t)(clear_mask >> 8), (uint8_t)(clear_mask) };

	return sx1262_write(p, tx, sizeof(tx));
}

bool sx1262_service_tx(sx1262_t *p)
{
	HAL_StatusTypeDef status;
	uint16_t irq = 0;

	status = sx1262_get_irq_status(p, &irq);
	if (status != HAL_OK) {
		SX1262_LOG(p, "sx1262: tx get irq status failed: %d\r\n", status);
		return false;
	}

	sx1262_clear_irq_status(p, SX1262_IRQ_ALL);

	if (irq & SX1262_IRQ_TX_DONE) {
		if (p->tx_done != NULL) {
			p->tx_done(p);
		}
	} else {
		SX1262_LOG(p, "sx1262: tx timeout (irq=0x%04X)\r\n", irq);

		if (p->tx_timeout != NULL) {
			p->tx_timeout(p);
		}
	}

	return (irq & SX1262_IRQ_TX_DONE) != 0;
}

bool sx1262_service_rx(sx1262_t *p)
{
	uint8_t payload[8] = { 0 };
	HAL_StatusTypeDef status;
	uint16_t irq = 0;
	int8_t rssi = 0;
	int8_t snr_quarter_db = 0;

	status = sx1262_get_irq_status(p, &irq);
	if (status != HAL_OK) {
		SX1262_LOG(p, "sx1262: rx get irq status failed: %d\r\n", status);
		return false;
	}

	if (irq & SX1262_IRQ_RX_DONE) {
		status = sx1262_read_buffer(p, 0, payload, sizeof(payload));
		if (status != HAL_OK) {
			SX1262_LOG(p, "sx1262: rx read buffer failed: %d\r\n", status);
		} else if (sx1262_get_packet_status(p, &rssi, &snr_quarter_db) == HAL_OK) {
			int snr_centi_db = (int)snr_quarter_db * 25;
			int snr_neg = (snr_centi_db < 0);

			if (snr_neg) {
				snr_centi_db = -snr_centi_db;
			}

			SX1262_LOG(p, "sx1262: rx done, payload=\"%.8s\", rssi=%ddBm, snr=%s%d.%02ddB\r\n",
				payload, rssi, snr_neg ? "-" : "", snr_centi_db / 100, snr_centi_db % 100);
		} else {
			SX1262_LOG(p, "sx1262: rx done, payload=\"%.8s\"\r\n", payload);
		}

		if (p->rx_done != NULL) {
			p->rx_done(p, payload, sizeof(payload), rssi, snr_quarter_db);
		}
	} else {
		SX1262_LOG(p, "sx1262: rx timeout (irq=0x%04X)\r\n", irq);

		if (p->rx_timeout != NULL) {
			p->rx_timeout(p);
		}
	}

	sx1262_clear_irq_status(p, SX1262_IRQ_ALL);

	return (irq & SX1262_IRQ_RX_DONE) != 0;
}

HAL_StatusTypeDef sx1262_get_device_errors(sx1262_t *p, uint16_t *out_errors)
{
	uint8_t tx[4] = { SX1262_OP_GET_DEVICE_ERRORS, SX1262_OP_NOP, SX1262_OP_NOP, SX1262_OP_NOP };
	uint8_t rx[4] = { 0 };
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

	*out_errors = ((uint16_t)rx[2] << 8) | rx[3];

	return HAL_OK;
}

HAL_StatusTypeDef sx1262_clear_device_errors(sx1262_t *p)
{
	uint8_t tx[3] = { SX1262_OP_CLEAR_DEVICE_ERRORS, 0x00, 0x00 };

	return sx1262_write(p, tx, sizeof(tx));
}
