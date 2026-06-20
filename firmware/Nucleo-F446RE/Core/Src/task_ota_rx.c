#include "task_ota_rx.h"
#include "task_logger.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "main.h"

#include "sx1262.h"
#include "ota_rx.h"
#include "config.h"
#include "ssd1309.h"

#include <stdio.h>
#include <string.h>

extern SemaphoreHandle_t hi2c1_mutex;

static SemaphoreHandle_t dio1_sem;

static sx1262_t  sx1262;
static ota_rx_t  ota_rx;
static config_t  config;
static ssd1309_t oled;

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
	if (gpio_pin == SX1262_SPI_DIO1_Pin) {
		BaseType_t woken = pdFALSE;
		xSemaphoreGiveFromISR(dio1_sem, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

static void on_rtcm3_frame(const uint8_t *frame, uint16_t len)
{
	uint16_t msg_type = ((uint16_t)frame[3] << 4) | (frame[4] >> 4);
	logger_log("ota_rx: frame msg=%u len=%u\r\n", (unsigned)msg_type, (unsigned)len);
	HAL_UART_Transmit(&huart3, (uint8_t *)frame, len, 200);
}

static void ota_rx_task(void *arg)
{
	(void)arg;

	vTaskDelay(pdMS_TO_TICKS(500));

	config_init(&config, &hi2c1, CONFIG_I2C_ADDR);
	config_set_log_callback(&config, logger_log);
	config_load(&config);

	xSemaphoreTake(hi2c1_mutex, portMAX_DELAY);
	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);
	ssd1309_bringup(&oled);
	xSemaphoreGive(hi2c1_mutex);

	sx1262_init(
		&sx1262, &hspi2,
		SX1262_SPI_CS_GPIO_Port,    SX1262_SPI_CS_Pin,
		SX1262_SPI_RESET_GPIO_Port, SX1262_SPI_RESET_Pin,
		SX1262_SPI_BUSY_GPIO_Port,  SX1262_SPI_BUSY_Pin
	);

	if (sx1262_config_gfsk(&sx1262, 434000000UL, 50000, 25000,
	                        OTA_RX_PACKET_SIZE, 0) != HAL_OK) {
		logger_log("sx1262: config gfsk failed\r\n");
	}

	ota_rx_init(&ota_rx, &sx1262, on_rtcm3_frame);
	sx1262_set_rx(&sx1262, SX1262_RX_TIMEOUT_CONTINUOUS);

	logger_log("F446RE Rover FreeRTOS starting\r\n");

	for (;;) {
		if (xSemaphoreTake(dio1_sem, pdMS_TO_TICKS(5000)) == pdTRUE) {
			sx1262_service_rx(&sx1262);
			sx1262_set_rx(&sx1262, SX1262_RX_TIMEOUT_CONTINUOUS);

			int snr_centi = (int)ota_rx.last_snr_quarter_db * 25;
			int snr_neg   = snr_centi < 0;
			if (snr_neg) snr_centi = -snr_centi;

			char line[24];
			xSemaphoreTake(hi2c1_mutex, portMAX_DELAY);
			ssd1309_clear(&oled);
			snprintf(line, sizeof(line), "RSSI: %ddBm", (int)ota_rx.last_rssi);
			ssd1309_draw_string(&oled, &font8x8, 0, 0, line, SSD1309_COLOR_ON);
			snprintf(line, sizeof(line), "SNR: %s%d.%02ddB",
			         snr_neg ? "-" : "", snr_centi / 100, snr_centi % 100);
			ssd1309_draw_string(&oled, &font8x8, 0, 12, line, SSD1309_COLOR_ON);
			ssd1309_flush(&oled);
			xSemaphoreGive(hi2c1_mutex);
		} else {
			logger_log("ota_rx: no packets for 5s\r\n");
			xSemaphoreTake(hi2c1_mutex, portMAX_DELAY);
			ssd1309_clear(&oled);
			ssd1309_draw_string(&oled, &font8x8, 0, 0, "No signal", SSD1309_COLOR_ON);
			ssd1309_flush(&oled);
			xSemaphoreGive(hi2c1_mutex);
		}
	}
}

void task_ota_rx_init(void)
{
	dio1_sem = xSemaphoreCreateBinary();
	configASSERT(dio1_sem);
	xTaskCreate(ota_rx_task, "ota_rx", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
}
