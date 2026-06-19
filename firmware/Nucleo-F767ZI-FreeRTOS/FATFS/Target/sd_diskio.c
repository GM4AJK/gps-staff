/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sd_diskio.c
  * @brief   SD Disk I/O driver
  *
  * NOTE: This file has been intentionally rewritten as part of the SD card
  * integration redesign.  The CubeMX-generated function bodies have been
  * replaced with a state-machine-driven implementation in task_sdcard.c.
  * CubeMX regeneration will overwrite the function bodies and must be
  * followed by re-applying this file from version control.
  *
  * Step 5: DMA read/write via HAL_SD_ReadBlocks_DMA/WriteBlocks_DMA with
  * xSemaphoreTake on sd_xfer_done.  BSP completion callbacks give the
  * semaphore from ISR.  Cortex-M7 D-cache maintenance applied per transfer.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "bsp_driver_sd.h"
#include "stm32f7xx_hal.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <string.h>

/* Sector size fixed at 512 for all supported cards. */
#define SD_DEFAULT_BLOCK_SIZE  512U
#define BLOCKSIZE              SD_DEFAULT_BLOCK_SIZE

/* Scratch buffer for the non-32-byte-aligned slow path.
 * Cortex-M7 D-cache maintenance (SCB_Clean/Invalidate) requires buffers to
 * be 32-byte aligned and sized to a multiple of the 32-byte cache line.
 * 512 bytes = 16 cache lines — exactly aligned. */
static uint8_t scratch[BLOCKSIZE] __attribute__((aligned(32)));

/* USER CODE BEGIN beforeFunctionSection */
/* hsd2 is owned by main.c / MX_SDMMC2_SD_Init; fully initialised by the
 * state machine before disk_initialize is ever called. */
extern SD_HandleTypeDef hsd2;

/* sd_xfer_done / sd_xfer_error live in task_sdcard.c.
 * sd_xfer_done is a binary semaphore: given from ISR by BSP callbacks,
 * taken in task context by disk_read / disk_write. */
extern SemaphoreHandle_t sd_xfer_done;
extern volatile uint8_t  sd_xfer_error;
/* USER CODE END beforeFunctionSection */

/* Private function prototypes -----------------------------------------------*/
static DSTATUS SD_initialize(BYTE lun);
static DSTATUS SD_status(BYTE lun);
static DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
static DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count);
#endif
#if _USE_IOCTL == 1
static DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff);
#endif

const Diskio_drvTypeDef SD_Driver = {
	SD_initialize,
	SD_status,
	SD_read,
#if _USE_WRITE == 1
	SD_write,
#endif
#if _USE_IOCTL == 1
	SD_ioctl,
#endif
};

/* Private functions ---------------------------------------------------------*/

/* ── disk_initialize / disk_status ──────────────────────────────────────── */
/* SDMMC is already initialised by the state machine before f_mount is called.
 * Confirm the card is in TRANSFER state by sending CMD13. */

/* USER CODE BEGIN beforeInitSection */
/* USER CODE END beforeInitSection */
static DSTATUS SD_initialize(BYTE lun)
{
	(void)lun;
	return (BSP_SD_GetCardState() == SD_TRANSFER_OK) ? 0 : STA_NOINIT;
}

/* USER CODE BEGIN beforeStatusSection */
/* USER CODE END beforeStatusSection */
static DSTATUS SD_status(BYTE lun)
{
	(void)lun;
	return (BSP_SD_GetCardState() == SD_TRANSFER_OK) ? 0 : STA_NOINIT;
}

/* ── disk_read (DMA) ─────────────────────────────────────────────────────── */
/* Cortex-M7 D-cache maintenance:
 *   Invalidate destination BEFORE DMA — clears any stale cached data so the
 *   CPU sees the DMA result from memory, not from a prefetched cache line.
 * The scratch buffer is used for any destination that is not 32-byte aligned
 * (required so cache line maintenance doesn't corrupt adjacent memory). */

/* USER CODE BEGIN beforeReadSection */
/* USER CODE END beforeReadSection */
static DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	(void)lun;

	for (UINT i = 0; i < count; i++) {
		uint8_t *dst  = buff + (size_t)i * BLOCKSIZE;
		uint8_t *rdst = ((uint32_t)dst & 31u) ? scratch : dst;

		SCB_InvalidateDCache_by_Addr((uint32_t *)rdst, BLOCKSIZE);

		sd_xfer_error = 0;
		if (HAL_SD_ReadBlocks_DMA(&hsd2, rdst, sector + i, 1) != HAL_OK)
			return RES_ERROR;

		if (xSemaphoreTake(sd_xfer_done, pdMS_TO_TICKS(5000)) != pdTRUE)
			return RES_ERROR;

		if (sd_xfer_error)
			return RES_ERROR;

		if (rdst != dst)
			memcpy(dst, scratch, BLOCKSIZE);
	}
	return RES_OK;
}

/* ── disk_write (DMA) ────────────────────────────────────────────────────── */
/* Cortex-M7 D-cache maintenance:
 *   Clean (write-back) source BEFORE DMA — flushes dirty cache lines to
 *   memory so DMA reads the data the CPU actually wrote.
 * After DMA completes the card may still be programming flash (PRG state);
 * poll BSP_SD_GetCardState before returning so the next command is safe. */

/* USER CODE BEGIN beforeWriteSection */
/* USER CODE END beforeWriteSection */
#if _USE_WRITE == 1
static DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	(void)lun;

	for (UINT i = 0; i < count; i++) {
		const uint8_t *src  = buff + (size_t)i * BLOCKSIZE;
		uint8_t       *wbuf = ((uint32_t)src & 31u) ? scratch : (uint8_t *)src;

		if (wbuf == scratch)
			memcpy(scratch, src, BLOCKSIZE);

		SCB_CleanDCache_by_Addr((uint32_t *)wbuf, BLOCKSIZE);

		sd_xfer_error = 0;
		if (HAL_SD_WriteBlocks_DMA(&hsd2, wbuf, sector + i, 1) != HAL_OK)
			return RES_ERROR;

		if (xSemaphoreTake(sd_xfer_done, pdMS_TO_TICKS(5000)) != pdTRUE)
			return RES_ERROR;

		if (sd_xfer_error)
			return RES_ERROR;

		/* Wait for card to finish internal flash programming (PRG -> TRANSFER). */
		uint32_t t = HAL_GetTick();
		for (;;) {
			if (BSP_SD_GetCardState() == SD_TRANSFER_OK) break;
			if ((HAL_GetTick() - t) >= 5000)            return RES_ERROR;
		}
	}
	return RES_OK;
}
#endif

/* ── disk_ioctl ──────────────────────────────────────────────────────────── */

/* USER CODE BEGIN beforeIoctlSection */
/* USER CODE END beforeIoctlSection */
#if _USE_IOCTL == 1
static DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	(void)lun;

	if (BSP_SD_GetCardState() != SD_TRANSFER_OK)
		return RES_NOTRDY;

	HAL_SD_CardInfoTypeDef info;
	switch (cmd) {
	case CTRL_SYNC:
		return RES_OK;
	case GET_SECTOR_COUNT:
		HAL_SD_GetCardInfo(&hsd2, &info);
		*(DWORD *)buff = info.LogBlockNbr;
		return RES_OK;
	case GET_SECTOR_SIZE:
		*(WORD *)buff = BLOCKSIZE;
		return RES_OK;
	case GET_BLOCK_SIZE:
		HAL_SD_GetCardInfo(&hsd2, &info);
		*(DWORD *)buff = info.LogBlockSize / BLOCKSIZE;
		return RES_OK;
	default:
		return RES_PARERR;
	}
}
#endif

/* USER CODE BEGIN callbackSection */
/* USER CODE END callbackSection */

/* ── BSP completion callbacks ─────────────────────────────────────────────── */
/* Strong overrides of the weak stubs in bsp_driver_sd.c.
 * Called in ISR context from HAL_SD_TxCpltCallback / RxCpltCallback / AbortCallback.
 * Give the binary semaphore to unblock the waiting disk_read / disk_write. */

void BSP_SD_WriteCpltCallback(void)
{
	if (sd_xfer_done) {
		BaseType_t woken = pdFALSE;
		xSemaphoreGiveFromISR(sd_xfer_done, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

void BSP_SD_ReadCpltCallback(void)
{
	if (sd_xfer_done) {
		BaseType_t woken = pdFALSE;
		xSemaphoreGiveFromISR(sd_xfer_done, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

/* USER CODE BEGIN ErrorAbortCallbacks */
void BSP_SD_AbortCallback(void)
{
	sd_xfer_error = 1;
	if (sd_xfer_done) {
		BaseType_t woken = pdFALSE;
		xSemaphoreGiveFromISR(sd_xfer_done, &woken);
		portYIELD_FROM_ISR(woken);
	}
}
/* USER CODE END ErrorAbortCallbacks */

/* USER CODE BEGIN lastSection */
/* USER CODE END lastSection */
