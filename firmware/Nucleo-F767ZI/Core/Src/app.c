

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "flags.h"
#include "ssd1309.h"

static ssd1309_t oled;

#define COUNTER_TIMER(x, y, z) \
	x++; \
	if(x >= y) { \
		x = 0; \
		z(); \
	}

void app_1ms(void)
{
	static volatile int cnt_10ms = 0;
	static volatile int cnt_100ms = 0;
	static volatile int cnt_1000ms = 0;
	COUNTER_TIMER(   cnt_10ms,   10, flag_set_10MS   );
	COUNTER_TIMER(  cnt_100ms,  100, flag_set_100MS  );
	COUNTER_TIMER( cnt_1000ms, 1000, flag_set_1000MS );
}

void app_init(void)
{
	ssd1309_init(&oled, &hi2c1, 0x3C, -1, -1);

	if (ssd1309_bringup(&oled) != HAL_OK) {
		const char *msg = "ssd1309_bringup failed\r\n";
		HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
		return;
	}

	ssd1309_clear(&oled);
	ssd1309_draw_string(&oled, &font5x7, 0, 0, "Nucleo-F767ZI OK", SSD1309_COLOR_ON);
	ssd1309_draw_line(&oled, 0, 9, oled.width - 1, 9, SSD1309_COLOR_ON);

	if (ssd1309_flush(&oled) != HAL_OK) {
		const char *msg = "ssd1309_flush failed\r\n";
		HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), 100);
	}
}

void app_loop(void)
{
	bool flipper = false;
	uint32_t loop_count = 0;
	char buf[48];

	while(true) {
		HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, flipper);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, !flipper);
		flipper = !flipper;

		int len = snprintf(buf, sizeof(buf), "loop %lu\r\n", loop_count++);
		HAL_UART_Transmit(&huart3, (uint8_t *)buf, len, 100);

		HAL_Delay(500);
	}
}
