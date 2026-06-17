
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "flags.h"
#include "ssd1309.h"
#include "Tests/test_ssd1309.h"
#include "sx1262.h"
#include "ota_rx.h"
#include "config.h"

static void app_tests(void);

static ssd1309_t  oled;
static sx1262_t   sx1262;
static ota_rx_t   ota_rx;
static config_t   config;

static uint8_t  rtcm3_pending_buf[OTA_RX_FRAME_BUF_SIZE];
static uint16_t rtcm3_pending_len;
static uint8_t  rtcm3_tx_buf[OTA_RX_FRAME_BUF_SIZE];
static volatile bool rtcm3_tx_busy;

void HAL_GPIO_EXTI_Callback(uint16_t gpio_pin)
{
	if (gpio_pin == SX1262_SPI_DIO1_Pin) {
		flag_set_SX1262_DIO1();
	}
}

#define COUNTER_TIMER(x, y, z) \
	static volatile int x = 0; \
	x++; \
	if(x >= y) { \
		x = 0; \
		z(); \
	}

void app_1ms(void)
{
	COUNTER_TIMER(   cnt_10ms,   10, flag_set_10MS   );
	COUNTER_TIMER(  cnt_100ms,  100, flag_set_100MS  );
	COUNTER_TIMER(  cnt_200ms,  200, flag_set_200MS  );
	COUNTER_TIMER(  cnt_500ms,  500, flag_set_500MS  );
}

void app_log(const char *fmt, ...)
{
	char buf[128];
	va_list args;

	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	HAL_UART_Transmit(&huart2, (uint8_t *)buf, len, 100);
}

#ifdef SX1262_WITH_LOGGING
static void sx1262_logger(const char *buf, int len)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)buf, len, 100);
}
#endif /* SX1262_WITH_LOGGING */

static void on_rtcm3_frame(const uint8_t *frame, uint16_t len)
{
	/* RTCM3 frame: [0xD3][len_hi][len_lo][msg_hi][msg_lo_and_data...][crc×3]
	 * Message type is the first 12 bits of the payload, i.e. frame[3..4]. */
	uint16_t msg_type = ((uint16_t)frame[3] << 4) | (frame[4] >> 4);
	app_log("ota_rx: frame msg=%u len=%u\r\n", (unsigned)msg_type, (unsigned)len);
	/* Don't transmit here — this is called from inside sx1262_service_rx.
	 * Copy to pending buffer; app_loop copies to tx_buf and starts IT after
	 * the radio is re-armed. rtcm3_tx_buf is owned by the UART until
	 * HAL_UART_TxCpltCallback clears rtcm3_tx_busy. */
	if(len <= sizeof(rtcm3_pending_buf)) {
		memcpy(rtcm3_pending_buf, frame, len);
		rtcm3_pending_len = len;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart3) rtcm3_tx_busy = false;
}

void app_init(void)
{
	/* Enable DWT cycle counter for timing diagnostics */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

	config_init(&config, &hi2c1, CONFIG_I2C_ADDR);
	config_set_log_callback(&config, app_log);
	config_load(&config);

	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);

	if (ssd1309_bringup(&oled) != HAL_OK) {
		app_log("ssd1309_bringup failed\r\n");
		return;
	}

	sx1262_init(
		&sx1262, &hspi2,
		SX1262_SPI_CS_GPIO_Port, SX1262_SPI_CS_Pin,
		SX1262_SPI_RESET_GPIO_Port, SX1262_SPI_RESET_Pin,
		SX1262_SPI_BUSY_GPIO_Port, SX1262_SPI_BUSY_Pin
	);

#ifdef SX1262_WITH_LOGGING
	sx1262_set_logger_callback(&sx1262, sx1262_logger);
#endif /* SX1262_WITH_LOGGING */

	if(sx1262_config_gfsk(&sx1262, 434000000UL, 50000, 25000, OTA_RX_PACKET_SIZE, 0) != HAL_OK) {
		app_log("sx1262: config gfsk failed\r\n");
		return;
	}

	ota_rx_init(&ota_rx, &sx1262, on_rtcm3_frame);

	app_log("F446RE Rover Start up\r\n");

	app_tests();
}

void app_loop(void)
{
	bool flipper = false;

	sx1262_set_rx(&sx1262, SX1262_RX_TIMEOUT_CONTINUOUS);

	while(true) {
		if(flag_get_100MS()) {
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, flipper);
			flipper = !flipper;
		}

		if(flag_get_SX1262_DIO1()) {
			sx1262_service_rx(&sx1262);
			sx1262_set_rx(&sx1262, SX1262_RX_TIMEOUT_CONTINUOUS);
		}

		if(rtcm3_pending_len > 0 && !rtcm3_tx_busy) {
			memcpy(rtcm3_tx_buf, rtcm3_pending_buf, rtcm3_pending_len);
			rtcm3_tx_busy = true;
			HAL_UART_Transmit_IT(&huart3, rtcm3_tx_buf, rtcm3_pending_len);
			rtcm3_pending_len = 0;
		}
	}
}

static void app_tests(void)
{
#ifdef TEST_SSD1309
	HAL_StatusTypeDef r = test_ssd1309_shapes(&oled);
	if (r != HAL_OK) {
		app_log("ssd1309_flush failed: %d\r\n", r);
	}
#endif /* TEST_SSD1309 */
}
