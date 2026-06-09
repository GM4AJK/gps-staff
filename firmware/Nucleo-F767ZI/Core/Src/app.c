

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "flags.h"

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
	// TDo
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
