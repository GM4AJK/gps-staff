

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"
#include "flags.h"
#include "ssd1309.h"
#include "Tests/test_ssd1309.h"
#include "bno085.h"
#include "Tests/test_bno085.h"

static void app_tests(void);

static ssd1309_t oled;
static bno085_t bno085;

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
	COUNTER_TIMER(  cnt_200ms,  100, flag_set_200MS  );
	COUNTER_TIMER(  cnt_500ms,  100, flag_set_500MS  );
	COUNTER_TIMER( cnt_1000ms, 1000, flag_set_1000MS );
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

void app_init(void)
{
	/* Enable the DWT cycle counter (CYCCNT) for microsecond-resolution timing */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);

	if (ssd1309_bringup(&oled) != HAL_OK) {
		app_log("ssd1309_bringup failed\r\n");
		return;
	}

	bno085_init(&bno085, &hi2c1, BNO085_I2C_ADDRESS);

#ifdef TEST_BNO085
	test_bno085_rotation_vector_enable(&bno085);
#endif /* TEST_BNO085 */

	app_log("Start up\r\n");

	app_tests();
}

void app_loop(void)
{
	static uint32_t exec_us = 0;

	while(true) {
		uint32_t start_time = DWT->CYCCNT;
		test_bno085_rotation_vector_display(&bno085, &oled, exec_us);
		uint32_t cycles = DWT->CYCCNT - start_time;
		exec_us = cycles / (HAL_RCC_GetHCLKFreq() / 1000000);

		if(flag_get_500MS()) {
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
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

#ifdef TEST_BNO085
	test_bno085_hello(&bno085);
	test_bno085_product_id(&bno085);
	test_bno085_rotation_vector(&bno085);
#endif /* TEST_BNO085 */
}
