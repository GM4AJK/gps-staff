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
  * Step 4: polling read/write via HAL_SD_ReadBlocks/WriteBlocks.
  * Step 5 will replace these with HAL_SD_ReadBlocks_DMA/WriteBlocks_DMA
  * plus an xfer_done binary semaphore for non-blocking DMA operation.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "bsp_driver_sd.h"
#include "stm32f7xx_hal.h"

#include <string.h>

/* Sector size is fixed at 512 for all supported cards. */
#define SD_DEFAULT_BLOCK_SIZE  512U
#define BLOCKSIZE              SD_DEFAULT_BLOCK_SIZE

/* Scratch buffer for unaligned-address slow path.
 * HAL_SD_ReadBlocks/WriteBlocks require 4-byte aligned buffers.
 * Step 5 DMA cache maintenance also requires 32-byte alignment. */
static uint8_t scratch[BLOCKSIZE] __attribute__((aligned(32)));

/* USER CODE BEGIN beforeFunctionSection */
/* hsd2 is initialised by the state machine before disk_initialize is called. */
extern SD_HandleTypeDef hsd2;
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
 * Confirm the card is in TRANSFER state by sending CMD13 via BSP_SD_GetCardState. */

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

/* ── disk_read (Step 4: polling via HAL_SD_ReadBlocks) ───────────────────── */
/* CPU reads directly from SDMMC RXFIFO — no DMA, no D-cache coherency issue.
 * Step 5 replaces this with DMA + xSemaphoreTake. */

/* USER CODE BEGIN beforeReadSection */
/* USER CODE END beforeReadSection */
static DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	(void)lun;

	for (UINT i = 0; i < count; i++) {
		uint8_t *dst  = buff + (size_t)i * BLOCKSIZE;
		/* HAL polling read requires 4-byte aligned destination. */
		uint8_t *rdst = ((uint32_t)dst & 3u) ? scratch : dst;

		if (HAL_SD_ReadBlocks(&hsd2, rdst, sector + i, 1, 5000) != HAL_OK)
			return RES_ERROR;

		/* Wait for card to return to TRANSFER state before the next command.
		 * SDMMC DTIMER fires on card removal so this doesn't spin for 5 s. */
		uint32_t t = HAL_GetTick();
		for (;;) {
			if (BSP_SD_GetCardState() == SD_TRANSFER_OK) break;
			if ((HAL_GetTick() - t) >= 5000)            return RES_ERROR;
		}

		if (rdst != dst)
			memcpy(dst, scratch, BLOCKSIZE);
	}
	return RES_OK;
}

/* ── disk_write (Step 4: polling via HAL_SD_WriteBlocks) ─────────────────── */
/* Step 5 replaces this with DMA + xSemaphoreTake. */

/* USER CODE BEGIN beforeWriteSection */
/* USER CODE END beforeWriteSection */
#if _USE_WRITE == 1
static DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	(void)lun;

	for (UINT i = 0; i < count; i++) {
		const uint8_t *src  = buff + (size_t)i * BLOCKSIZE;
		uint8_t       *wbuf = ((uint32_t)src & 3u) ? scratch : (uint8_t *)src;

		if (wbuf == scratch)
			memcpy(scratch, src, BLOCKSIZE);

		if (HAL_SD_WriteBlocks(&hsd2, wbuf, sector + i, 1, 5000) != HAL_OK)
			return RES_ERROR;

		/* Card enters PRG state during internal flash write; poll until ready. */
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

/* BSP callbacks — strong overrides of the weak stubs in bsp_driver_sd.c.
 * Called in ISR context via HAL_SD_TxCpltCallback / RxCpltCallback / AbortCallback.
 * Step 5: give xfer_done semaphore (set xfer_error on abort) from ISR. */

void BSP_SD_WriteCpltCallback(void) {}
void BSP_SD_ReadCpltCallback(void)  {}

/* USER CODE BEGIN ErrorAbortCallbacks */
void BSP_SD_AbortCallback(void) {}
/* USER CODE END ErrorAbortCallbacks */

/* USER CODE BEGIN lastSection */
/* USER CODE END lastSection */
