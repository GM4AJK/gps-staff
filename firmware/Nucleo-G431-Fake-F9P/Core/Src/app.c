
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "flags.h"
#include "rtcm_sample.h"

#define RTCM_SYNC_LEN 6

typedef enum {
	F9P_ROLE_ROVER = 0,
	F9P_ROLE_BASE  = 1,
} f9p_role_t;

static f9p_role_t role;

#define COUNTER_TIMER(x, y, z) \
	static volatile int x = 0; \
	x++; \
	if(x >= y) { \
		x = 0; \
		z(); \
	}

void app_1ms(void)
{
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
	role = (HAL_GPIO_ReadPin(F9P_MODE_SELECT_GPIO_Port, F9P_MODE_SELECT_Pin) == GPIO_PIN_SET)
		? F9P_ROLE_BASE
		: F9P_ROLE_ROVER;

	app_log("Fake F9P start up, role = %s\r\n", (role == F9P_ROLE_BASE) ? "BASE" : "ROVER");
}

static void base_loop(void)
{
	uint32_t cycle = 0;

	while(true) {
		if(flag_get_1000MS()) {
			uint32_t start = rtcm_sample_cycle_offsets[cycle];
			uint32_t end = (cycle + 1 < rtcm_sample_cycle_count)
				? rtcm_sample_cycle_offsets[cycle + 1]
				: rtcm_sample_len;

			HAL_UART_Transmit(&huart1, (uint8_t *)&rtcm_sample_data[start], end - start, 200);
			app_log("base: tx cycle=%lu bytes=%lu\r\n", (unsigned long)cycle, (unsigned long)(end - start));
			HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

			cycle = (cycle + 1) % rtcm_sample_cycle_count;
		}
	}
}

static void rover_loop(void)
{
	uint8_t rx_byte;
	uint8_t sync_hist[RTCM_SYNC_LEN] = {0};
	uint32_t rx_index = 0;
	bool synced = false;
	uint32_t total_bytes = 0;
	uint32_t mismatches = 0;
	uint32_t resyncs = 0;

	while(true) {
		if(HAL_UART_Receive(&huart1, &rx_byte, 1, 10) == HAL_OK) {
			total_bytes++;

			if(!synced) {
				memmove(sync_hist, sync_hist + 1, RTCM_SYNC_LEN - 1);
				sync_hist[RTCM_SYNC_LEN - 1] = rx_byte;
				if(memcmp(sync_hist, rtcm_sample_data, RTCM_SYNC_LEN) == 0) {
					synced = true;
					rx_index = RTCM_SYNC_LEN % rtcm_sample_len;
					resyncs++;
				}
			} else if(rx_byte != rtcm_sample_data[rx_index]) {
				mismatches++;
				synced = false;
				memset(sync_hist, 0, sizeof(sync_hist));
			} else {
				rx_index = (rx_index + 1) % rtcm_sample_len;
			}
		}

		if(flag_get_1000MS()) {
			app_log("rover: %s total=%lu mismatches=%lu resyncs=%lu\r\n",
				synced ? "SYNCED" : "SEARCH",
				(unsigned long)total_bytes, (unsigned long)mismatches, (unsigned long)resyncs);
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, synced ? GPIO_PIN_SET : GPIO_PIN_RESET);
		}
	}
}

void app_loop(void)
{
	if(role == F9P_ROLE_BASE) {
		base_loop();
	} else {
		rover_loop();
	}
}
