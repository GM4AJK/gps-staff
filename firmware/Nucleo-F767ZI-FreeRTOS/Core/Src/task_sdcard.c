#include "task_sdcard.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "fatfs.h"
#include "bsp_driver_sd.h"
#include "task_logger.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

extern SD_HandleTypeDef hsd2;

/* Declared in sd_diskio.c USER CODE sections. */
extern void SD_abort_notify(void);
extern void SD_flush_queue(void);

/* DWT cycle counter: wraps at ~19.9 s at 216 MHz.
 * Good enough for relative timing; use the deltas, not absolute values. */
static inline uint32_t ts_ms(void)
{
	return DWT->CYCCNT / (SystemCoreClock / 1000U);
}

/* Staging latch for hsd2.ErrorCode captured inside HAL_SD_ErrorCallback (ISR).
 * Read and cleared from task context so we can log it without blocking the ISR. */
static volatile uint32_t sd_hal_error = 0;

/* DWT timestamp latched by SD_abort_notify (ISR context) so we can log it from
 * task context without calling logger from an ISR.  Set to 0 after logging. */
volatile uint32_t sd_abort_ts_ms = 0;

/* Diagnostic probes: DWT timestamps set at key points inside f_mount, read
 * after f_mount returns.  Both reset to 0 after being logged so each attempt
 * starts fresh.
 * diag_bsi_enter: set at BSP_SD_Init entry (0 = BSP_SD_Init never called)
 * diag_sdr_enter: set at SD_read entry  (0 = disk_read never reached)
 * Both written from task context; diag_sdr_enter also written from sd_diskio.c. */
volatile uint32_t diag_bsi_enter = 0;
volatile uint32_t diag_sdr_enter = 0;

/* Strong override: capture the SDMMC error code and immediately unblock any
 * SD_read or SD_write that is blocking in osMessageQueueGet.  Without this,
 * the DMA completion callback never fires on error and the task spins 30 s
 * waiting for READ_CPLT_MSG / WRITE_CPLT_MSG that will never arrive. */
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
	sd_hal_error = hsd->ErrorCode;
	SD_abort_notify();
}

/* Strong override: RCC-reset SDMMC2 before every init to clear any stuck DMA
 * or DPSM state left by a hot-eject, then do a full DeInit + Init cycle.
 * After a successful reinit: drain SDQueueID of stale abort messages so the
 * next disk_read is not immediately poisoned by a leftover RW_ABORT_MSG. */
uint8_t BSP_SD_Init(void)
{
	diag_bsi_enter = ts_ms();	/* must be first: non-droppable timestamp probe */
	logger_log("[%lu] sdcard: BSP_SD_Init enter\r\n", ts_ms());
	if (BSP_SD_IsDetected() != SD_PRESENT) {
		logger_log("[%lu] sdcard: BSP_SD_IsDetected not present\r\n", ts_ms());
		return MSD_ERROR_SD_NOT_PRESENT;
	}
	__HAL_RCC_SDMMC2_FORCE_RESET();
	HAL_Delay(2);
	__HAL_RCC_SDMMC2_RELEASE_RESET();
	HAL_SD_DeInit(&hsd2);
	logger_log("[%lu] sdcard: HAL_SD_Init start\r\n", ts_ms());
	HAL_StatusTypeDef r = HAL_SD_Init(&hsd2);
	if (r != HAL_OK) {
		logger_log("[%lu] sdcard: HAL_SD_Init failed state=%d err=0x%08lx\r\n",
		           ts_ms(), (int)hsd2.State, (unsigned long)hsd2.ErrorCode);
		return MSD_ERROR;
	}
	sd_hal_error = 0;
	SD_flush_queue();
	logger_log("[%lu] sdcard: BSP_SD_Init OK state=%d\r\n", ts_ms(), (int)hsd2.State);
	return MSD_OK;
}

/* Strong override: log HAL_SD_ReadBlocks_DMA failures only. */
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks)
{
	if (HAL_SD_ReadBlocks_DMA(&hsd2, (uint8_t *)pData, ReadAddr, NumOfBlocks) != HAL_OK) {
		logger_log("[%lu] sdcard: ReadBlocks_DMA FAILED addr=%lu state=%d err=0x%08lx\r\n",
		           ts_ms(), (unsigned long)ReadAddr,
		           (int)hsd2.State, (unsigned long)hsd2.ErrorCode);
		return MSD_ERROR;
	}
	return MSD_OK;
}

#define SD_QUEUE_DEPTH  3

typedef struct {
	uint8_t  data[SD_FRAME_MAX];
	uint16_t len;
} sd_frame_t;

static QueueHandle_t sd_q;

void task_sdcard_push_frame(const uint8_t *data, uint16_t len)
{
	if (!sd_q || len > SD_FRAME_MAX)
		return;
	static sd_frame_t buf;
	buf.len = len;
	memcpy(buf.data, data, len);
	xQueueSend(sd_q, &buf, 0);
}

static bool open_next_file(FIL *fp)
{
	char path[13];
	for (unsigned n = 1; n <= 9999; n++) {
		snprintf(path, sizeof(path), "RTCM%04u.BIN", n);
		if (f_stat(path, NULL) != FR_OK) {
			if (f_open(fp, path, FA_CREATE_NEW | FA_WRITE) == FR_OK) {
				logger_log("[%lu] sdcard: logging to %s\r\n", ts_ms(), path);
				return true;
			}
		}
	}
	logger_log("[%lu] sdcard: no free filename slot\r\n", ts_ms());
	return false;
}

static FATFS    fs;
static FIL      fp;

static void sdcard_task(void *arg)
{
	(void)arg;
	bool     mounted   = false;
	bool     file_open = false;
	uint32_t frames    = 0;
	static sd_frame_t f;

	for (;;) {
		if (!mounted) {
			if (BSP_SD_IsDetected() != SD_PRESENT) {
				vTaskDelay(pdMS_TO_TICKS(1000));
				continue;
			}
			uint32_t abort_ts = sd_abort_ts_ms;
			if (abort_ts) {
				logger_log("[%lu] sdcard: abort callback fired at %lu ms\r\n",
				           ts_ms(), abort_ts);
				sd_abort_ts_ms = 0;
			}
			logger_log("[%lu] sdcard: calling f_mount\r\n", ts_ms());
			FRESULT r = f_mount(&fs, "", 1);
			/* Single post-f_mount diagnostic: bsi/sdr are non-droppable volatile
			 * timestamps that survive even if logger_log inside f_mount was dropped.
			 * bsi=0 → BSP_SD_Init never called; sdr=0 → disk_read never reached. */
			logger_log("[%lu] sdcard: f_mount=%d bsi=%lu sdr=%lu hst=%d herr=0x%lx\r\n",
			           ts_ms(), (int)r, diag_bsi_enter, diag_sdr_enter,
			           (int)hsd2.State, (unsigned long)hsd2.ErrorCode);
			diag_bsi_enter = 0;
			diag_sdr_enter = 0;
			if (r != FR_OK) {
				vTaskDelay(pdMS_TO_TICKS(2000));
				continue;
			}
			mounted = true;
			logger_log("[%lu] sdcard: mounted\r\n", ts_ms());
		}

		if (!file_open) {
			if (!open_next_file(&fp)) {
				vTaskDelay(pdMS_TO_TICKS(5000));
				continue;
			}
			file_open = true;
			frames    = 0;
		}

		if (xQueueReceive(sd_q, &f, pdMS_TO_TICKS(1000)) != pdTRUE)
			continue;

		UINT written;
		FRESULT r = f_write(&fp, f.data, f.len, &written);
		if (r != FR_OK || written != f.len) {
			logger_log("[%lu] sdcard: write error (%d), remounting\r\n", ts_ms(), (int)r);
			f_close(&fp);
			f_mount(NULL, "", 0);
			file_open = false;
			mounted   = false;
			continue;
		}
		f_sync(&fp);
		frames++;
		if (frames % 100 == 0)
			logger_log("[%lu] sdcard: %lu frames written\r\n",
			           ts_ms(), (unsigned long)frames);
	}
}

void task_sdcard_init(void)
{
	sd_q = xQueueCreate(SD_QUEUE_DEPTH, sizeof(sd_frame_t));
	configASSERT(sd_q);
	xTaskCreate(sdcard_task, "sdcard", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
}
