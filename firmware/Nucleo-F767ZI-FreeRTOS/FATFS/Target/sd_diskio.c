/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    sd_diskio.c
  * @brief   SD Disk I/O driver — DMA + FreeRTOS semaphore implementation.
  *
  * The CubeMX-generated function bodies (osMessageQueue approach) are silently
  * redirected to _cx_* dead-code symbols via #define macros in
  * beforeFunctionSection.  Real implementations live in lastSection.  All
  * USER CODE sections survive CubeMX regeneration, so re-applying this file
  * from version control is NOT required after a regen.
  *
  * See docs/arch/sdcard.md for full design documentation.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Note: code generation based on sd_diskio_dma_rtos_template_bspv1.c v2.1.4
   as FreeRTOS is enabled. */

/* USER CODE BEGIN firstSection */
/* Bare FreeRTOS project — stub the cmsis_os types used in the CubeMX-generated
 * function bodies below.  Those bodies are redirected to _cx_* dead code by
 * #define macros in beforeFunctionSection; they must still compile. */
#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "bsp_driver_sd.h"

#define osCMSIS 0x00001U	/* force v1 branch so only one set of stubs needed */
typedef void *osMessageQId;
typedef struct { int status; struct { uint32_t v; } value; } osEvent;
#define osEventMessage 0x02
static inline int          osKernelRunning(void)                                 { return 1; }
static inline uint32_t     osKernelSysTick(void)                                 { return 0; }
static inline osMessageQId _osmc_stub(void)                                      { return NULL; }
#define osMessageQDef(name, queue_sz, type)
#define osMessageQ(name)                      NULL
#define osMessageCreate(qdef, thread_id)      _osmc_stub()
static inline osEvent osMessageGet(osMessageQId q, uint32_t ms)                  { (void)q; (void)ms; osEvent e = {0}; return e; }
static inline int     osMessagePut(osMessageQId q, uint32_t i, uint32_t ms)      { (void)q; (void)i; (void)ms; return 0; }
/* USER CODE END firstSection*/

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"
#include "sd_diskio.h"

#include <string.h>
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define QUEUE_SIZE         (uint32_t) 10
#define READ_CPLT_MSG      (uint32_t) 1
#define WRITE_CPLT_MSG     (uint32_t) 2
/*
==================================================================
enable the defines below to send custom rtos messages
when an error or an abort occurs.
Notice: depending on the HAL/SD driver the HAL_SD_ErrorCallback()
may not be available.
See BSP_SD_ErrorCallback() and BSP_SD_AbortCallback() below
==================================================================

#define RW_ERROR_MSG       (uint32_t) 3
#define RW_ABORT_MSG       (uint32_t) 4
*/
/*
 * the following Timeout is useful to give the control back to the applications
 * in case of errors in either BSP_SD_ReadCpltCallback() or BSP_SD_WriteCpltCallback()
 * the value by default is as defined in the BSP platform driver otherwise 30 secs
 */
#define SD_TIMEOUT 30 * 1000

#define SD_DEFAULT_BLOCK_SIZE 512

/*
 * Depending on the use case, the SD card initialization could be done at the
 * application level: if it is the case define the flag below to disable
 * the BSP_SD_Init() call in the SD_Initialize() and add a call to
 * BSP_SD_Init() elsewhere in the application.
 */
/* USER CODE BEGIN disableSDInit */
/* #define DISABLE_SD_INIT */
/* USER CODE END disableSDInit */

/*
 * when using cacheable memory region, it may be needed to maintain the cache
 * validity. Enable the define below to activate a cache maintenance at each
 * read and write operation.
 * Notice: This is applicable only for cortex M7 based platform.
 */
/* USER CODE BEGIN enableSDDmaCacheMaintenance */
/* #define ENABLE_SD_DMA_CACHE_MAINTENANCE  1 */
/* USER CODE END enableSDDmaCacheMaintenance */

/*
* Some DMA requires 4-Byte aligned address buffer to correctly read/write data,
* in FatFs some accesses aren't thus we need a 4-byte aligned scratch buffer to correctly
* transfer data
*/
/* USER CODE BEGIN enableScratchBuffer */
/* #define ENABLE_SCRATCH_BUFFER */
/* USER CODE END enableScratchBuffer */

/* Private variables ---------------------------------------------------------*/
#if defined(ENABLE_SCRATCH_BUFFER)
#if defined (ENABLE_SD_DMA_CACHE_MAINTENANCE)
ALIGN_32BYTES(static uint8_t scratch[BLOCKSIZE]); // 32-Byte aligned for cache maintenance
#else
__ALIGN_BEGIN static uint8_t scratch[BLOCKSIZE] __ALIGN_END;
#endif
#endif
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

#if (osCMSIS <= 0x20000U)
static osMessageQId SDQueueID = NULL;
#else
static osMessageQueueId_t SDQueueID = NULL;
#endif
/* Private function prototypes -----------------------------------------------*/
static DSTATUS SD_CheckStatus(BYTE lun);
DSTATUS SD_initialize (BYTE);
DSTATUS SD_status (BYTE);
DRESULT SD_read (BYTE, BYTE*, DWORD, UINT);
#if _USE_WRITE == 1
DRESULT SD_write (BYTE, const BYTE*, DWORD, UINT);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
DRESULT SD_ioctl (BYTE, BYTE, void*);
#endif  /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef  SD_Driver =
{
  SD_initialize,
  SD_status,
  SD_read,
#if  _USE_WRITE == 1
  SD_write,
#endif /* _USE_WRITE == 1 */

#if  _USE_IOCTL == 1
  SD_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* USER CODE BEGIN beforeFunctionSection */
/* ── Redirect CubeMX-generated function bodies to _cx_* dead code ───────────
 * The SD_Driver dispatch table above already captured the real names before
 * these macros were defined.  Everything between here and lastSection compiles
 * as _cx_* symbols that are never referenced and will be stripped by the
 * linker.  Real implementations live in lastSection below.
 * On CubeMX regen the macros survive (they are inside USER CODE) and the
 * newly regenerated bodies are again silently redirected. */
#define SD_CheckStatusWithTimeout   _cx_SD_CheckStatusWithTimeout
#define SD_CheckStatus              _cx_SD_CheckStatus
#define SD_initialize               _cx_SD_initialize
#define SD_status                   _cx_SD_status
#define SD_read                     _cx_SD_read
#define SD_write                    _cx_SD_write
#define SD_ioctl                    _cx_SD_ioctl
#define BSP_SD_WriteCpltCallback    _cx_BSP_SD_WriteCpltCallback
#define BSP_SD_ReadCpltCallback     _cx_BSP_SD_ReadCpltCallback

#define BLOCKSIZE SD_DEFAULT_BLOCK_SIZE

/* 32-byte aligned scratch buffer for DMA transfers to/from buffers that are
 * not 32-byte aligned (Cortex-M7 cache line size = 32 bytes). */
static uint8_t scratch[BLOCKSIZE] __attribute__((aligned(32)));

/* hsd2 is owned by main.c / MX_SDMMC2_SD_Init; fully initialised by the
 * state machine before disk_initialize is ever called. */
extern SD_HandleTypeDef hsd2;

/* sd_xfer_done / sd_xfer_error live in task_sdcard.c.
 * sd_xfer_done is a binary semaphore: given from ISR by BSP callbacks,
 * taken in task context by disk_read / disk_write. */
extern SemaphoreHandle_t sd_xfer_done;
extern volatile uint8_t  sd_xfer_error;
/* USER CODE END beforeFunctionSection */

/* Private functions ---------------------------------------------------------*/

static int SD_CheckStatusWithTimeout(uint32_t timeout)
{
  uint32_t timer;
  /* block until SDIO peripheral is ready again or a timeout occur */
#if (osCMSIS <= 0x20000U)
  timer = osKernelSysTick();
  while( osKernelSysTick() - timer < timeout)
#else
  timer = osKernelGetTickCount();
  while( osKernelGetTickCount() - timer < timeout)
#endif
  {
    if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
    {
      return 0;
    }
  }

  return -1;
}

static DSTATUS SD_CheckStatus(BYTE lun)
{
  Stat = STA_NOINIT;

  if(BSP_SD_GetCardState() == SD_TRANSFER_OK)
  {
    Stat &= ~STA_NOINIT;
  }

  return Stat;
}

/**
  * @brief  Initializes a Drive
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_initialize(BYTE lun)
{
Stat = STA_NOINIT;

  /*
   * check that the kernel has been started before continuing
   * as the osMessage API will fail otherwise
   */
#if (osCMSIS <= 0x20000U)
  if(osKernelRunning())
#else
  if(osKernelGetState() == osKernelRunning)
#endif
  {
#if !defined(DISABLE_SD_INIT)

    if(BSP_SD_Init() == MSD_OK)
    {
      Stat = SD_CheckStatus(lun);
    }

#else
    Stat = SD_CheckStatus(lun);
#endif

    /*
    * if the SD is correctly initialized, create the operation queue
    * if not already created
    */

    if (Stat != STA_NOINIT)
    {
      if (SDQueueID == NULL)
      {
 #if (osCMSIS <= 0x20000U)
      osMessageQDef(SD_Queue, QUEUE_SIZE, uint16_t);
      SDQueueID = osMessageCreate (osMessageQ(SD_Queue), NULL);
#else
      SDQueueID = osMessageQueueNew(QUEUE_SIZE, 2, NULL);
#endif
      }

      if (SDQueueID == NULL)
      {
        Stat |= STA_NOINIT;
      }
    }
  }

  return Stat;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS SD_status(BYTE lun)
{
  return SD_CheckStatus(lun);
}

/* USER CODE BEGIN beforeReadSection */
/* USER CODE END beforeReadSection */
/**
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */

DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
  uint8_t ret;
  DRESULT res = RES_ERROR;
  uint32_t timer;
#if (osCMSIS < 0x20000U)
  osEvent event;
#else
  uint16_t event;
  osStatus_t status;
#endif
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
  uint32_t alignedAddr;
#endif
  /*
  * ensure the SDCard is ready for a new operation
  */

  if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0)
  {
    return res;
  }

#if defined(ENABLE_SCRATCH_BUFFER)
  if (!((uint32_t)buff & 0x3))
  {
#endif
    /* Fast path cause destination buffer is correctly aligned */
    ret = BSP_SD_ReadBlocks_DMA((uint32_t*)buff, (uint32_t)(sector), count);

    if (ret == MSD_OK) {
#if (osCMSIS < 0x20000U)
    /* wait for a message from the queue or a timeout */
    event = osMessageGet(SDQueueID, SD_TIMEOUT);

    if (event.status == osEventMessage)
    {
      if (event.value.v == READ_CPLT_MSG)
      {
        timer = osKernelSysTick();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelSysTick() - timer <SD_TIMEOUT)
#else
          status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
          if ((status == osOK) && (event == READ_CPLT_MSG))
          {
            timer = osKernelGetTickCount();
            /* block until SDIO IP is ready or a timeout occur */
            while(osKernelGetTickCount() - timer <SD_TIMEOUT)
#endif
            {
              if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
              {
                res = RES_OK;
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
                /*
                the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
                adjust the address and the D-Cache size to invalidate accordingly.
                */
                alignedAddr = (uint32_t)buff & ~0x1F;
                SCB_InvalidateDCache_by_Addr((uint32_t*)alignedAddr, count*BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif
                break;
              }
            }
#if (osCMSIS < 0x20000U)
          }
        }
#else
      }
#endif
    }

#if defined(ENABLE_SCRATCH_BUFFER)
    }
    else
    {
      /* Slow path, fetch each sector a part and memcpy to destination buffer */
      int i;

      for (i = 0; i < count; i++)
      {
        ret = BSP_SD_ReadBlocks_DMA((uint32_t*)scratch, (uint32_t)sector++, 1);
        if (ret == MSD_OK )
        {
          /* wait until the read is successful or a timeout occurs */
#if (osCMSIS < 0x20000U)
          /* wait for a message from the queue or a timeout */
          event = osMessageGet(SDQueueID, SD_TIMEOUT);

          if (event.status == osEventMessage)
          {
            if (event.value.v == READ_CPLT_MSG)
            {
              timer = osKernelSysTick();
              /* block until SDIO IP is ready or a timeout occur */
              while(osKernelSysTick() - timer <SD_TIMEOUT)
#else
                status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
              if ((status == osOK) && (event == READ_CPLT_MSG))
              {
                timer = osKernelGetTickCount();
                /* block until SDIO IP is ready or a timeout occur */
                ret = MSD_ERROR;
                while(osKernelGetTickCount() - timer < SD_TIMEOUT)
#endif
                {
                  ret = BSP_SD_GetCardState();

                  if (ret == MSD_OK)
                  {
                    break;
                  }
                }

                if (ret != MSD_OK)
                {
                  break;
                }
#if (osCMSIS < 0x20000U)
              }
            }
#else
          }
#endif
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
          /*
          *
          * invalidate the scratch buffer before the next read to get the actual data instead of the cached one
          */
          SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, BLOCKSIZE);
#endif
          memcpy(buff, scratch, BLOCKSIZE);
          buff += BLOCKSIZE;
        }
        else
        {
          break;
        }
      }

      if ((i == count) && (ret == MSD_OK ))
        res = RES_OK;
    }
#endif
  return res;
}

/* USER CODE BEGIN beforeWriteSection */
/* USER CODE END beforeWriteSection */
/**
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1

DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;
  uint32_t timer;

#if (osCMSIS < 0x20000U)
  osEvent event;
#else
  uint16_t event;
  osStatus_t status;
#endif

#if defined(ENABLE_SCRATCH_BUFFER)
  int32_t ret;
#endif

  /*
  * ensure the SDCard is ready for a new operation
  */

  if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0)
  {
    return res;
  }

#if defined(ENABLE_SCRATCH_BUFFER)
  if (!((uint32_t)buff & 0x3))
  {
#endif
#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
  uint32_t alignedAddr;
  /*
    the SCB_CleanDCache_by_Addr() requires a 32-Byte aligned address
    adjust the address and the D-Cache size to clean accordingly.
  */
  alignedAddr = (uint32_t)buff & ~0x1F;
  SCB_CleanDCache_by_Addr((uint32_t*)alignedAddr, count*BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif

  if(BSP_SD_WriteBlocks_DMA((uint32_t*)buff,
                           (uint32_t) (sector),
                           count) == MSD_OK)
  {
#if (osCMSIS < 0x20000U)
    /* Get the message from the queue */
    event = osMessageGet(SDQueueID, SD_TIMEOUT);

    if (event.status == osEventMessage)
    {
      if (event.value.v == WRITE_CPLT_MSG)
      {
#else
    status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
    if ((status == osOK) && (event == WRITE_CPLT_MSG))
    {
#endif
 #if (osCMSIS < 0x20000U)
        timer = osKernelSysTick();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelSysTick() - timer  < SD_TIMEOUT)
#else
        timer = osKernelGetTickCount();
        /* block until SDIO IP is ready or a timeout occur */
        while(osKernelGetTickCount() - timer  < SD_TIMEOUT)
#endif
        {
          if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
          {
            res = RES_OK;
            break;
          }
        }
#if (osCMSIS < 0x20000U)
      }
    }
#else
    }
#endif
  }
#if defined(ENABLE_SCRATCH_BUFFER)
  }
  else {
    /* Slow path, fetch each sector a part and memcpy to destination buffer */
    int i;

#if (ENABLE_SD_DMA_CACHE_MAINTENANCE == 1)
    /*
     * invalidate the scratch buffer before the next write to get the actual data instead of the cached one
     */
     SCB_InvalidateDCache_by_Addr((uint32_t*)scratch, BLOCKSIZE);
#endif
      for (i = 0; i < count; i++)
      {
        memcpy((void *)scratch, buff, BLOCKSIZE);
        buff += BLOCKSIZE;

        ret = BSP_SD_WriteBlocks_DMA((uint32_t*)scratch, (uint32_t)sector++, 1);
        if (ret == MSD_OK )
        {
          /* wait until the read is successful or a timeout occurs */
#if (osCMSIS < 0x20000U)
          /* wait for a message from the queue or a timeout */
          event = osMessageGet(SDQueueID, SD_TIMEOUT);

          if (event.status == osEventMessage)
          {
            if (event.value.v == READ_CPLT_MSG)
            {
              timer = osKernelSysTick();
              /* block until SDIO IP is ready or a timeout occur */
              while(osKernelSysTick() - timer <SD_TIMEOUT)
#else
                status = osMessageQueueGet(SDQueueID, (void *)&event, NULL, SD_TIMEOUT);
              if ((status == osOK) && (event == READ_CPLT_MSG))
              {
                timer = osKernelGetTickCount();
                /* block until SDIO IP is ready or a timeout occur */
                ret = MSD_ERROR;
                while(osKernelGetTickCount() - timer < SD_TIMEOUT)
#endif
                {
                  ret = BSP_SD_GetCardState();

                  if (ret == MSD_OK)
                  {
                    break;
                  }
                }

                if (ret != MSD_OK)
                {
                  break;
                }
#if (osCMSIS < 0x20000U)
              }
            }
#else
          }
#endif
        }
        else
        {
          break;
        }
      }

      if ((i == count) && (ret == MSD_OK ))
        res = RES_OK;
    }

#endif

  return res;
}
 #endif /* _USE_WRITE == 1 */

/* USER CODE BEGIN beforeIoctlSection */
/* USER CODE END beforeIoctlSection */
/**
  * @brief  I/O control operation
  * @param  lun : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  BSP_SD_CardInfo CardInfo;

  if (Stat & STA_NOINIT) return RES_NOTRDY;

  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;

  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    BSP_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockNbr;
    res = RES_OK;
    break;

  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    BSP_SD_GetCardInfo(&CardInfo);
    *(WORD*)buff = CardInfo.LogBlockSize;
    res = RES_OK;
    break;

  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    BSP_SD_GetCardInfo(&CardInfo);
    *(DWORD*)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
    res = RES_OK;
    break;

  default:
    res = RES_PARERR;
  }

  return res;
}
#endif /* _USE_IOCTL == 1 */

/* USER CODE BEGIN afterIoctlSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END afterIoctlSection */

/* USER CODE BEGIN callbackSection */
/* USER CODE END callbackSection */
/**
  * @brief Tx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_WriteCpltCallback(void)
{

  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
#if (osCMSIS < 0x20000U)
   osMessagePut(SDQueueID, WRITE_CPLT_MSG, 0);
#else
   const uint16_t msg = WRITE_CPLT_MSG;
   osMessageQueuePut(SDQueueID, (const void *)&msg, 0, 0);
#endif
}

/**
  * @brief Rx Transfer completed callbacks
  * @param hsd: SD handle
  * @retval None
  */
void BSP_SD_ReadCpltCallback(void)
{
  /*
   * No need to add an "osKernelRunning()" check here, as the SD_initialize()
   * is always called before any SD_Read()/SD_Write() call
   */
#if (osCMSIS < 0x20000U)
   osMessagePut(SDQueueID, READ_CPLT_MSG, 0);
#else
   const uint16_t msg = READ_CPLT_MSG;
   osMessageQueuePut(SDQueueID, (const void *)&msg, 0, 0);
#endif
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
/* ── Undo CubeMX redirect macros; real implementations follow ────────────── */
#undef SD_CheckStatusWithTimeout
#undef SD_CheckStatus
#undef SD_initialize
#undef SD_status
#undef SD_read
#undef SD_write
#undef SD_ioctl
#undef BSP_SD_WriteCpltCallback
#undef BSP_SD_ReadCpltCallback

static DSTATUS SD_CheckStatus(BYTE lun)
{
	(void)lun;
	Stat = STA_NOINIT;
	if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
		Stat &= ~STA_NOINIT;
	return Stat;
}

DSTATUS SD_initialize(BYTE lun)
{
	Stat = SD_CheckStatus(lun);
	return Stat;
}

DSTATUS SD_status(BYTE lun)
{
	return SD_CheckStatus(lun);
}

DRESULT SD_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
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

#if _USE_WRITE == 1
DRESULT SD_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
	(void)lun;
	for (UINT i = 0; i < count; i++) {
		const uint8_t *src  = buff + (size_t)i * BLOCKSIZE;
		uint8_t       *wbuf = ((uint32_t)src & 31u) ? scratch : (uint8_t *)src;
		if (wbuf == scratch) memcpy(scratch, src, BLOCKSIZE);
		SCB_CleanDCache_by_Addr((uint32_t *)wbuf, BLOCKSIZE);
		sd_xfer_error = 0;
		if (HAL_SD_WriteBlocks_DMA(&hsd2, wbuf, sector + i, 1) != HAL_OK)
			return RES_ERROR;
		if (xSemaphoreTake(sd_xfer_done, pdMS_TO_TICKS(5000)) != pdTRUE)
			return RES_ERROR;
		if (sd_xfer_error)
			return RES_ERROR;
		uint32_t t = HAL_GetTick();
		for (;;) {
			if (BSP_SD_GetCardState() == SD_TRANSFER_OK) break;
			if ((HAL_GetTick() - t) >= 5000) return RES_ERROR;
		}
	}
	return RES_OK;
}
#endif /* _USE_WRITE == 1 */

#if _USE_IOCTL == 1
DRESULT SD_ioctl(BYTE lun, BYTE cmd, void *buff)
{
	(void)lun;
	DRESULT res = RES_ERROR;
	BSP_SD_CardInfo CardInfo;

	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (cmd) {
	case CTRL_SYNC:
		res = RES_OK;
		break;
	case GET_SECTOR_COUNT:
		BSP_SD_GetCardInfo(&CardInfo);
		*(DWORD *)buff = CardInfo.LogBlockNbr;
		res = RES_OK;
		break;
	case GET_SECTOR_SIZE:
		BSP_SD_GetCardInfo(&CardInfo);
		*(WORD *)buff = CardInfo.LogBlockSize;
		res = RES_OK;
		break;
	case GET_BLOCK_SIZE:
		BSP_SD_GetCardInfo(&CardInfo);
		*(DWORD *)buff = CardInfo.LogBlockSize / SD_DEFAULT_BLOCK_SIZE;
		res = RES_OK;
		break;
	default:
		res = RES_PARERR;
	}
	return res;
}
#endif /* _USE_IOCTL == 1 */

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
/* USER CODE END lastSection */
