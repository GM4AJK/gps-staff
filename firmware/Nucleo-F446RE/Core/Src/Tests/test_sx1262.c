#include "Tests/test_sx1262.h"
#include "app.h"

#include <stdio.h>
#include <string.h>

#ifdef TEST_SX1262

static ssd1309_t *oled = NULL;

void test_sx1262_set_oled(ssd1309_t *p)
{
	oled = p;
}

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

	/* Waveshare Core1262-LF: 32MHz reference is a TCXO powered via DIO3 */
	status = sx1262_set_dio3_as_tcxo_ctrl(p, SX1262_TCXO_VOLTAGE_1_8, 320);
	if (status != HAL_OK) {
		app_log("sx1262: set dio3 as tcxo ctrl failed: %d\r\n", status);
		return;
	}

	status = sx1262_clear_device_errors(p);
	if (status != HAL_OK) {
		app_log("sx1262: clear device errors failed: %d\r\n", status);
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

	status = sx1262_set_pa_config(p, 0x02, 0x03, SX1262_PA_CONFIG_SX1262);
	if (status != HAL_OK) {
		app_log("sx1262: set pa config failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_tx_params(p, 17, SX1262_RAMP_200U);
	if (status != HAL_OK) {
		app_log("sx1262: set tx params failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_dio_irq_params(p, SX1262_IRQ_ALL, SX1262_IRQ_TX_DONE | SX1262_IRQ_RX_DONE | SX1262_IRQ_TIMEOUT, 0, 0);
	if (status != HAL_OK) {
		app_log("sx1262: set dio irq params failed: %d\r\n", status);
		return;
	}

	sx1262_set_rx_done_callback(p, test_sx1262_rx_done_handler);
	sx1262_set_tx_done_callback(p, test_sx1262_tx_done_toggle_led);

	app_log("sx1262: configured LoRa @ 434.000MHz, SF7/BW125/CR4_5, preamble=8 explicit CRC, +17dBm\r\n");
}

static uint8_t rx_payload[9] = { 0 };
static int8_t rx_rssi = 0;
static int8_t rx_snr_quarter_db = 0;

void test_sx1262_rx_done_handler(sx1262_t *p)
{
	char line[24];
	int snr_centi_db = (int)rx_snr_quarter_db * 25;
	int snr_neg = (snr_centi_db < 0);

	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

	if (oled == NULL) {
		return;
	}

	if (snr_neg) {
		snr_centi_db = -snr_centi_db;
	}

	ssd1309_clear(oled);

	snprintf(line, sizeof(line), "RX: %.8s", rx_payload);
	ssd1309_draw_string(oled, &font5x7, 0, 0, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "RSSI: %ddBm", rx_rssi);
	ssd1309_draw_string(oled, &font5x7, 0, 10, line, SSD1309_COLOR_ON);

	snprintf(line, sizeof(line), "SNR: %s%d.%02ddB", snr_neg ? "-" : "", snr_centi_db / 100, snr_centi_db % 100);
	ssd1309_draw_string(oled, &font5x7, 0, 20, line, SSD1309_COLOR_ON);

	ssd1309_flush(oled);
}

void test_sx1262_tx_done_toggle_led(sx1262_t *p)
{
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}

static uint8_t tx_payload[8] = "PING0000";

void test_sx1262_tx_start(sx1262_t *p)
{
	static uint8_t counter = 0;
	HAL_StatusTypeDef status;

	tx_payload[7] = '0' + counter;
	counter = (counter + 1) % 10;

	status = sx1262_write_buffer(p, 0, tx_payload, sizeof(tx_payload));
	if (status != HAL_OK) {
		app_log("sx1262: tx write buffer failed: %d\r\n", status);
		return;
	}

	status = sx1262_set_tx(p, SX1262_RX_TX_TIMEOUT_NONE);
	if (status != HAL_OK) {
		app_log("sx1262: set tx failed: %d\r\n", status);
		return;
	}
}

bool test_sx1262_tx_done(sx1262_t *p)
{
	HAL_StatusTypeDef status;
	uint16_t irq = 0;

	status = sx1262_get_irq_status(p, &irq);
	if (status != HAL_OK) {
		app_log("sx1262: tx get irq status failed: %d\r\n", status);
		return false;
	}

	sx1262_clear_irq_status(p, SX1262_IRQ_ALL);

	if (irq & SX1262_IRQ_TX_DONE) {
		app_log("sx1262: tx done, payload=\"%.8s\"\r\n", tx_payload);

		if (p->tx_done != NULL) {
			p->tx_done(p);
		}
	} else {
		app_log("sx1262: tx timeout (irq=0x%04X)\r\n", irq);
	}

	return (irq & SX1262_IRQ_TX_DONE) != 0;
}

void test_sx1262_rx_start(sx1262_t *p)
{
	HAL_StatusTypeDef status;

	status = sx1262_set_rx(p, 64000UL);
	if (status != HAL_OK) {
		app_log("sx1262: set rx failed: %d\r\n", status);
		return;
	}
}

bool test_sx1262_rx_done(sx1262_t *p)
{
	uint8_t payload[8] = { 0 };
	HAL_StatusTypeDef status;
	uint16_t irq = 0;

	status = sx1262_get_irq_status(p, &irq);
	if (status != HAL_OK) {
		app_log("sx1262: rx get irq status failed: %d\r\n", status);
		return false;
	}

	if (irq & SX1262_IRQ_RX_DONE) {
		status = sx1262_read_buffer(p, 0, payload, sizeof(payload));
		if (status != HAL_OK) {
			app_log("sx1262: rx read buffer failed: %d\r\n", status);
		} else {
			int8_t rssi = 0;
			int8_t snr_quarter_db = 0;

			if (sx1262_get_packet_status(p, &rssi, &snr_quarter_db) == HAL_OK) {
				int snr_centi_db = (int)snr_quarter_db * 25;
				int snr_neg = (snr_centi_db < 0);

				if (snr_neg) {
					snr_centi_db = -snr_centi_db;
				}

				app_log("sx1262: rx done, payload=\"%.8s\", rssi=%ddBm, snr=%s%d.%02ddB\r\n",
					payload, rssi, snr_neg ? "-" : "", snr_centi_db / 100, snr_centi_db % 100);

				rx_rssi = rssi;
				rx_snr_quarter_db = snr_quarter_db;
			} else {
				app_log("sx1262: rx done, payload=\"%.8s\"\r\n", payload);

				rx_rssi = 0;
				rx_snr_quarter_db = 0;
			}

			memcpy(rx_payload, payload, sizeof(payload));
		}
		if (p->rx_done != NULL) {
			p->rx_done(p);
		}
	} else {
		app_log("sx1262: rx timeout (irq=0x%04X)\r\n", irq);
	}

	sx1262_clear_irq_status(p, SX1262_IRQ_ALL);

	return (irq & SX1262_IRQ_RX_DONE) != 0;
}

#endif /* TEST_SX1262 */
