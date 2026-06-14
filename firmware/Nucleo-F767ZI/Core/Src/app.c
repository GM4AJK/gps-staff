

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"
#include "flags.h"
#include "ssd1309.h"
#include "Tests/test_ssd1309.h"
#include "sx1262.h"
#include "Tests/test_sx1262.h"

static void app_tests(void);

static ssd1309_t oled;
static sx1262_t sx1262;

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

void app_init(void)
{
	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

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

	app_log("Start up\r\n");

	app_tests();
}

void app_loop(void)
{
	bool flipper = false;

	while(true) {
		if(flag_get_100MS()) {
			HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, flipper);
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, !flipper);
			flipper = !flipper;
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
	test_sx1262_config(&sx1262);
#endif /* TEST_SX1262 */
}
