#include "Tests/test_sx1262.h"
#include "app.h"

#ifdef TEST_SX1262

void test_sx1262_hello(sx1262_t *p)
{
	HAL_StatusTypeDef status;
	uint8_t value;

	status = sx1262_reset(p);
	if (status != HAL_OK) {
		app_log("sx1262: reset failed: %d\r\n", status);
		return;
	}

	status = sx1262_get_status(p, &value);
	if (status != HAL_OK) {
		app_log("sx1262: get status failed: %d\r\n", status);
		return;
	}

	uint8_t chip_mode = (value >> 4) & 0x07;
	uint8_t cmd_status = (value >> 1) & 0x07;

	app_log("sx1262: status=0x%02X (chip mode=%u, cmd status=%u)\r\n", value, chip_mode, cmd_status);
}

void test_sx1262_config(sx1262_t *p)
{
	HAL_StatusTypeDef status;

	status = sx1262_reset(p);
	if (status != HAL_OK) {
		app_log("sx1262: reset failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_packet_type(p, SX1262_PACKET_TYPE_LORA);
	if (status != HAL_OK) {
		app_log("sx1262: set packet type failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_rf_frequency(p, 434000000UL);
	if (status != HAL_OK) {
		app_log("sx1262: set rf frequency failed: %d\r\n", status);
		return;
	}

	status = sx1262_calibrate_image(p, SX1262_CAL_IMG_430_440_FREQ1, SX1262_CAL_IMG_430_440_FREQ2);
	if (status != HAL_OK) {
		app_log("sx1262: calibrate image failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_modulation_params_lora(p, SX1262_LORA_SF7, SX1262_LORA_BW_125, SX1262_LORA_CR_4_5, SX1262_LORA_LDRO_OFF);
	if (status != HAL_OK) {
		app_log("sx1262: set modulation params failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_packet_params_lora(p, 8, SX1262_LORA_HEADER_EXPLICIT, 8, SX1262_LORA_CRC_ON, SX1262_LORA_IQ_STANDARD);
	if (status != HAL_OK) {
		app_log("sx1262: set packet params failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_buffer_base_address(p, 0, 0);
	if (status != HAL_OK) {
		app_log("sx1262: set buffer base address failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_pa_config(p, 0x02, 0x02, SX1262_PA_CONFIG_SX1262);
	if (status != HAL_OK) {
		app_log("sx1262: set pa config failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_tx_params(p, 14, SX1262_RAMP_200U);
	if (status != HAL_OK) {
		app_log("sx1262: set tx params failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_dio_irq_params(p, SX1262_IRQ_ALL, 0, 0, 0);
	if (status != HAL_OK) {
		app_log("sx1262: set dio irq params failed: %d\r\n", status);
		return;
	}

	app_log("sx1262: configured LoRa @ 434.000MHz, SF7/BW125/CR4_5, preamble=8 explicit CRC, +14dBm\r\n");
}

void test_sx1262_tx(sx1262_t *p)
{
	static uint8_t counter = 0;
	uint8_t payload[8] = "PING0000";
	HAL_StatusTypeDef status;
	uint16_t irq = 0;
	uint32_t start;

	payload[7] = '0' + counter;
	counter = (counter + 1) % 10;

	status = sx1262_write_buffer(p, 0, payload, sizeof(payload));
	if (status != HAL_OK) {
		app_log("sx1262: tx write buffer failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_tx(p, SX1262_RX_TX_TIMEOUT_NONE);
	if (status != HAL_OK) {
		app_log("sx1262: set tx failed: %d\r\n", status);
		return;
	}

	{
		uint8_t value = 0;
		uint16_t errors = 0;
		sx1262_get_status(p, &value);
		sx1262_get_device_errors(p, &errors);
		app_log("sx1262: tx status=0x%02X (chip mode=%u, cmd status=%u), errors=0x%04X\r\n", value, (value >> 4) & 0x07, (value >> 1) & 0x07, errors);
	}

	start = HAL_GetTick();
	do {
		status = sx1262_get_irq_status(p, &irq);
		if (status != HAL_OK) {
			app_log("sx1262: tx get irq status failed: %d\r\n", status);
			return;
		}
		if (irq & (SX1262_IRQ_TX_DONE | SX1262_IRQ_TIMEOUT)) {
			break;
		}
	} while ((HAL_GetTick() - start) < 2000);

	sx1262_clear_irq_status(p, SX1262_IRQ_ALL);

	if (irq & SX1262_IRQ_TX_DONE) {
		app_log("sx1262: tx done, payload=\"%.8s\"\r\n", payload);
	} else {
		app_log("sx1262: tx timeout (irq=0x%04X)\r\n", irq);
	}
}

void test_sx1262_rx(sx1262_t *p)
{
	uint8_t payload[8] = { 0 };
	HAL_StatusTypeDef status;
	uint16_t irq = 0;
	uint32_t start;

	status = sx1262_set_rx(p, 64000UL);
	if (status != HAL_OK) {
		app_log("sx1262: set rx failed: %d\r\n", status);
		return;
	}

	{
		uint8_t value = 0;
		uint16_t errors = 0;
		sx1262_get_status(p, &value);
		sx1262_get_device_errors(p, &errors);
		app_log("sx1262: rx status=0x%02X (chip mode=%u, cmd status=%u), errors=0x%04X\r\n", value, (value >> 4) & 0x07, (value >> 1) & 0x07, errors);
	}

	start = HAL_GetTick();
	do {
		status = sx1262_get_irq_status(p, &irq);
		if (status != HAL_OK) {
			app_log("sx1262: rx get irq status failed: %d\r\n", status);
			return;
		}
		if (irq & (SX1262_IRQ_RX_DONE | SX1262_IRQ_TIMEOUT)) {
			break;
		}
	} while ((HAL_GetTick() - start) < 1100);

	if (irq & SX1262_IRQ_RX_DONE) {
		status = sx1262_read_buffer(p, 0, payload, sizeof(payload));
		if (status != HAL_OK) {
			app_log("sx1262: rx read buffer failed: %d\r\n", status);
		} else {
			app_log("sx1262: rx done, payload=\"%.8s\"\r\n", payload);
		}
	} else {
		app_log("sx1262: rx timeout (irq=0x%04X)\r\n", irq);
	}

	sx1262_clear_irq_status(p, SX1262_IRQ_ALL);
}

#endif /* TEST_SX1262 */
