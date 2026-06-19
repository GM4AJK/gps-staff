#include "task_sdcard.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "bsp_driver_sd.h"
#include "fatfs.h"
#include "task_logger.h"

#include <stdio.h>
#include <string.h>

extern SD_HandleTypeDef hsd2;

static inline uint32_t ts_ms(void)
{
	return DWT->CYCCNT / (SystemCoreClock / 1000U);
}

/* ── Card detect debounce (ms_sd_card called from TIM6 ISR via main.c) ─── */

volatile uint8_t    sd_card_present = 0;		/* level: debounced card state  */
static uint8_t      sd_detect_count = 0;		/* insert debounce counter      */
static TaskHandle_t sd_task_handle  = NULL;		/* set by task at startup       */

void ms_sd_card(void)
{
	if (HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, SD_DETECT_PIN) == GPIO_PIN_RESET) {
		/* Active-low: GPIO_PIN_RESET means card physically present. */
		if (sd_detect_count < 20) {
			sd_detect_count++;
			if (sd_detect_count == 20) {
				sd_card_present = 1;
				if (sd_task_handle) {
					BaseType_t woken = pdFALSE;
					vTaskNotifyGiveFromISR(sd_task_handle, &woken);
					portYIELD_FROM_ISR(woken);
				}
			}
		}
	} else {
		/* Card absent or bouncing — clear flag and counter immediately.
		 * Notify task on the falling edge so it can log the transition. */
		if (sd_card_present) {
			sd_card_present = 0;
			if (sd_task_handle) {
				BaseType_t woken = pdFALSE;
				vTaskNotifyGiveFromISR(sd_task_handle, &woken);
					portYIELD_FROM_ISR(woken);
			}
		}
		sd_detect_count = 0;
	}
}

/* ── HAL error callback (forwarded from main.c HAL_SD_ErrorCallback) ────── */

void sdcard_on_hal_error(SD_HandleTypeDef *hsd)
{
	(void)hsd;
	/* Step 5: set xfer_error flag and give xfer_done semaphore from ISR. */
}

/* ── State machine ────────────────────────────────────────────────────────── */

typedef enum {
	SD_ABSENT,
	SD_INIT,
	SD_MOUNTING,
	SD_IDLE,
	SD_XFER,
} sd_state_t;

static FATFS   fs;
static FIL     fp;
static uint8_t mounted   = 0;
static uint8_t file_open = 0;

static uint8_t open_next_file(void)
{
	char path[13];
	for (unsigned n = 1; n <= 9999; n++) {
		snprintf(path, sizeof(path), "RTCM%04u.BIN", n);
		if (f_stat(path, NULL) != FR_OK) {
			if (f_open(&fp, path, FA_CREATE_NEW | FA_WRITE) == FR_OK) {
				logger_log("[%lu] sd: logging to %s\r\n", ts_ms(), path);
				return 1;
			}
		}
	}
	logger_log("[%lu] sd: no free filename slot\r\n", ts_ms());
	return 0;
}

/* Called on every transition to SD_ABSENT. Extended each step:
 * Step 5 will add f_sync before f_close. */
static void sd_teardown(void)
{
	if (file_open) { f_close(&fp); file_open = 0; }
	if (mounted)   { f_mount(NULL, "", 0); mounted = 0; }
	HAL_SD_DeInit(&hsd2);
	__HAL_RCC_SDMMC2_FORCE_RESET();
	HAL_Delay(2);
	__HAL_RCC_SDMMC2_RELEASE_RESET();
}

static void sdcard_task(void *arg)
{
	(void)arg;
	sd_task_handle = xTaskGetCurrentTaskHandle();

	sd_state_t state = SD_ABSENT;
	logger_log("[%lu] sd: ABSENT\r\n", ts_ms());

	for (;;) {
		switch (state) {

		case SD_ABSENT:
			ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
			if (sd_card_present) {
				logger_log("[%lu] sd: card present -> INIT\r\n", ts_ms());
				state = SD_INIT;
			} else {
				logger_log("[%lu] sd: card absent\r\n", ts_ms());
			}
			break;

		case SD_INIT:
			if (!sd_card_present) {
				logger_log("[%lu] sd: ejected before INIT -> ABSENT\r\n", ts_ms());
				state = SD_ABSENT;
				break;
			}
			__HAL_RCC_SDMMC2_FORCE_RESET();
			HAL_Delay(2);
			__HAL_RCC_SDMMC2_RELEASE_RESET();
			HAL_SD_DeInit(&hsd2);
			logger_log("[%lu] sd: HAL_SD_Init start\r\n", ts_ms());
			if (HAL_SD_Init(&hsd2) != HAL_OK) {
				logger_log("[%lu] sd: HAL_SD_Init FAIL state=%d err=0x%lx -> ABSENT\r\n",
				           ts_ms(), (int)hsd2.State, (unsigned long)hsd2.ErrorCode);
				sd_teardown();
				state = SD_ABSENT;
				break;
			}
			logger_log("[%lu] sd: HAL_SD_Init OK state=%d -> MOUNTING\r\n",
			           ts_ms(), (int)hsd2.State);
			state = SD_MOUNTING;
			break;

		case SD_MOUNTING: {
			FRESULT r = f_mount(&fs, "", 1);
			if (r != FR_OK) {
				logger_log("[%lu] sd: f_mount FAIL %d -> ABSENT\r\n", ts_ms(), (int)r);
				sd_teardown();
				state = SD_ABSENT;
				break;
			}
			mounted = 1;
			logger_log("[%lu] sd: f_mount OK\r\n", ts_ms());
			if (!open_next_file()) {
				sd_teardown();
				state = SD_ABSENT;
				break;
			}
			file_open = 1;
			logger_log("[%lu] sd: -> IDLE\r\n", ts_ms());
			state = SD_IDLE;
			break;
		}

		case SD_IDLE:
			/* Step 5: xQueueReceive + f_write + f_sync. */
			logger_log("[%lu] sd: IDLE stub -> ABSENT\r\n", ts_ms());
			sd_teardown();
			state = SD_ABSENT;
			break;

		case SD_XFER:
			/* Step 5: DMA write, blocked on semaphore. */
			logger_log("[%lu] sd: XFER stub -> ABSENT\r\n", ts_ms());
			sd_teardown();
			state = SD_ABSENT;
			break;
		}
	}
}

/* ── Public API ───────────────────────────────────────────────────────────── */

void task_sdcard_push_frame(const uint8_t *data, uint16_t len)
{
	/* Step 5: push frame to write queue. No-op until IDLE implemented. */
	(void)data;
	(void)len;
}

void task_sdcard_init(void)
{
	xTaskCreate(sdcard_task, "sdcard", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
}
