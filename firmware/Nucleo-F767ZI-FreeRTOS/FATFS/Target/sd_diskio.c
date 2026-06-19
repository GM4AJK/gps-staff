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
  ******************************************************************************
  */
/* USER CODE END Header */

/* USER CODE BEGIN firstSection */
/* USER CODE END firstSection */

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "bsp_driver_sd.h"

#include <string.h>

/* Private define ------------------------------------------------------------*/

#define SD_DEFAULT_BLOCK_SIZE  512

/*
 * Cache maintenance required on Cortex-M7: DMA bypasses D-cache so we must
 * clean (before write) and invalidate (after read) any cache lines that
 * overlap the DMA buffer.
 */
/* USER CODE BEGIN enableSDDmaCacheMaintenance */
#define ENABLE_SD_DMA_CACHE_MAINTENANCE  1
/* USER CODE END enableSDDmaCacheMaintenance */

/*
 * Scratch buffer for the unaligned-address slow path: FatFS occasionally
 * passes buffers that are not 4-byte aligned, so we bounce through here.
 */
/* USER CODE BEGIN enableScratchBuffer */
#define ENABLE_SCRATCH_BUFFER
/* USER CODE END enableScratchBuffer */

/* Private variables ---------------------------------------------------------*/
#if defined(ENABLE_SCRATCH_BUFFER)
ALIGN_32BYTES(static uint8_t scratch[BLOCKSIZE]);
#endif

/* USER CODE BEGIN beforeFunctionSection */
/* Step 5: declare xfer_done semaphore and xfer_error flag here. */
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

const Diskio_drvTypeDef SD_Driver =
{
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

/* USER CODE BEGIN beforeInitSection */
/* USER CODE END beforeInitSection */
static DSTATUS SD_initialize(BYTE lun)
{
	/* Step 3: call sdcard_hal_sd_init() which does RCC reset + HAL_SD_Init.
	 * Return STA_OK on success, STA_NOINIT on failure. */
	(void)lun;
	return STA_NOINIT;
}

/* USER CODE BEGIN beforeStatusSection */
/* USER CODE END beforeStatusSection */
static DSTATUS SD_status(BYTE lun)
{
	/* Step 3: return STA_OK when SDMMC is initialised, STA_NOINIT otherwise. */
	(void)lun;
	return STA_NOINIT;
}

/* USER CODE BEGIN beforeReadSection */
/* USER CODE END beforeReadSection */
static DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
	/* Step 5: semaphore-driven DMA read. */
	(void)lun; (void)buff; (void)sector; (void)count;
	return RES_NOTRDY;
}

/* USER CODE BEGIN beforeWriteSection */
/* USER CODE END beforeWriteSection */
#if _USE_WRITE == 1
static DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	/* Step 5: semaphore-driven DMA write. */
	(void)lun; (void)buff; (void)sector; (void)count;
	return RES_NOTRDY;
}
#endif

/* USER CODE BEGIN beforeIoctlSection */
/* USER CODE END beforeIoctlSection */
#if _USE_IOCTL == 1
static DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	/* Step 5: implement GET_SECTOR_COUNT, GET_SECTOR_SIZE, GET_BLOCK_SIZE,
	 * CTRL_SYNC. */
	(void)lun; (void)cmd; (void)buff;
	return RES_PARERR;
}
#endif

/* USER CODE BEGIN callbackSection */
/* USER CODE END callbackSection */

/* BSP callbacks — strong overrides of the weak stubs in bsp_driver_sd.c.
 * Called from HAL_SD_TxCpltCallback / HAL_SD_RxCpltCallback /
 * HAL_SD_AbortCallback (defined in bsp_driver_sd.c) in ISR context.
 * Step 5: give xfer_done semaphore and set xfer_error on abort. */

void BSP_SD_WriteCpltCallback(void)
{
	/* Step 5: xSemaphoreGiveFromISR(xfer_done, &woken); */
}

void BSP_SD_ReadCpltCallback(void)
{
	/* Step 5: xSemaphoreGiveFromISR(xfer_done, &woken); */
}

/* USER CODE BEGIN ErrorAbortCallbacks */
void BSP_SD_AbortCallback(void)
{
	/* Step 5: set xfer_error = true; xSemaphoreGiveFromISR(xfer_done, &woken); */
}
/* USER CODE END ErrorAbortCallbacks */

/* USER CODE BEGIN lastSection */
/* USER CODE END lastSection */
