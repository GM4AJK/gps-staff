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

	app_log("sx1262: configured LoRa @ 434.000MHz, SF7/BW125/CR4_5, preamble=8 explicit CRC\r\n");
}

#endif /* TEST_SX1262 */
