#pragma once
#include <stdint.h>
#include "stm32f7xx_hal.h"

/* Maximum RTCM3 frame size: protocol max 1023-byte payload + 6 framing bytes. */
#define SD_FRAME_MAX  1032

void task_sdcard_init(void);
void task_sdcard_push_frame(const uint8_t *data, uint16_t len);

/* Called from HAL_TIM_PeriodElapsedCallback in main.c (1 ms TIM6 tick). */
void ms_sd_card(void);

/* Called from HAL_SD_ErrorCallback in main.c. */
void sdcard_on_hal_error(SD_HandleTypeDef *hsd);
