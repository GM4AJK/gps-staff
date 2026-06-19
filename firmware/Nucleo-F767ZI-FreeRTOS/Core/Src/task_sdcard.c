#include "task_sdcard.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "bsp_driver_sd.h"
#include "task_logger.h"

#include <string.h>

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
				logger_log("[%lu] sd: card present → INIT\r\n", ts_ms());
				state = SD_INIT;
			} else {
				logger_log("[%lu] sd: card absent\r\n", ts_ms());
			}
			break;

		case SD_INIT:
			/* Step 3: RCC reset + HAL_SD_DeInit + HAL_SD_Init. */
			logger_log("[%lu] sd: INIT stub → ABSENT\r\n", ts_ms());
			state = SD_ABSENT;
			break;

		case SD_MOUNTING:
			/* Step 4: f_mount + open_next_file. */
			logger_log("[%lu] sd: MOUNTING stub → ABSENT\r\n", ts_ms());
			state = SD_ABSENT;
			break;

		case SD_IDLE:
			/* Step 5: xQueueReceive + f_write + f_sync. */
			logger_log("[%lu] sd: IDLE stub → ABSENT\r\n", ts_ms());
			state = SD_ABSENT;
			break;

		case SD_XFER:
			/* Step 5: DMA write, blocked on semaphore. */
			logger_log("[%lu] sd: XFER stub → ABSENT\r\n", ts_ms());
			state = SD_ABSENT;
			break;
		}
	}
}

/* ── Public API ───────────────────────────────────────────────────────────── */

void task_sdcard_push_frame(const uint8_t *data, uint16_t len)
{
	/* Step 5: push frame to write queue. No-op until IDLE/XFER implemented. */
	(void)data;
	(void)len;
}

void task_sdcard_init(void)
{
	xTaskCreate(sdcard_task, "sdcard", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
}
