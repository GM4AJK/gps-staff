

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"
#include "flags.h"
#include "rtcm3.h"
#include "ota.h"
#include "ssd1309.h"
#include "Tests/test_ssd1309.h"
#include "sx1262.h"
#include "Tests/test_sx1262.h"

static void app_tests(void);

rtcm3_t rtcm3;
static ota_t ota;
static ssd1309_t oled;
static sx1262_t sx1262;

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
	COUNTER_TIMER( cnt_1000ms, 1000, flag_set_1000MS );
}

void app_log(const char *fmt, ...)
{
	char buf[128];
	va_list args;

	va_start(args, fmt);
	int len = vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
}

#ifdef SX1262_WITH_LOGGING
static void sx1262_logger(const char *buf, int len)
{
	HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);
}
#endif /* SX1262_WITH_LOGGING */

static void on_rtcm3_frame(const uint8_t *frame, uint16_t len)
{
	ota_push_frame(&ota, frame, len);
}

void app_init(void)
{
	/* Enable DWT cycle counter for timing diagnostics.
	 * Cortex-M7's DWT block is behind a lock register - writes to CTRL/CYCCNT
	 * are silently ignored until it's unlocked with this magic value. */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->LAR = 0xC5ACCE55;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);

	if (ssd1309_bringup(&oled) != HAL_OK) {
		app_log("ssd1309_bringup failed\r\n");
		return;
	}

#ifdef TEST_SX1262
	test_sx1262_set_oled(&oled);
#endif /* TEST_SX1262 */

	sx1262_init(
		&sx1262, &hspi2,
		SX1262_SPI_CS_GPIO_Port, SX1262_SPI_CS_Pin,
		SX1262_SPI_RESET_GPIO_Port, SX1262_SPI_RESET_Pin,
		SX1262_SPI_BUSY_GPIO_Port, SX1262_SPI_BUSY_Pin
	);

#ifdef SX1262_WITH_LOGGING
	sx1262_set_logger_callback(&sx1262, sx1262_logger);
#endif /* SX1262_WITH_LOGGING */

	rtcm3_init(&rtcm3, &huart2, on_rtcm3_frame);
	ota_init(&ota, &sx1262);

	app_log("Start up\r\n");

	app_tests();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2) {
		rtcm3_uart_in_irq(&rtcm3);
	}
}

void app_loop(void)
{
	bool flipper = false;

	while(true) {
		rtcm3_loop(&rtcm3);

		if(flag_get_100MS()) {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, flipper);
			flipper = !flipper;
		}

		if(flag_get_SX1262_DIO1()) {
			sx1262_service_tx(&sx1262);
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

#ifdef TEST_SX1262
#ifdef TEST_SX1262_GFSK
	test_sx1262_config_gfsk(&sx1262);
#else
	test_sx1262_config(&sx1262);
#endif /* TEST_SX1262_GFSK */
#endif /* TEST_SX1262 */
}
