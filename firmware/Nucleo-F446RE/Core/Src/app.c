

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "main.h"
#include "flags.h"

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

	HAL_UART_Transmit(&huart2, (uint8_t *)buf, len, 100);
}

void app_init(void)
{
	/* Allow externally connected devices time to power up before init */
	HAL_Delay(500);

	app_log("Start up\r\n");
}

void app_loop(void)
{
	while(true) {
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		HAL_Delay(250);
	}
}
