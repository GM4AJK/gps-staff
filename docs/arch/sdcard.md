# SD Card Module

## Purpose

The SD card module records every valid RTCM3 frame received from the F9P
(or Fake-F9P) to a binary file on a µSD card fitted to the Nucleo-F767ZI
base station.  Each mount cycle creates a new sequentially-numbered file
(`RTCM0001.BIN`, `RTCM0002.BIN`, …) so data from separate sessions is never
overwritten.  The files can be replayed offline to validate OTA transmission
or post-process RTK corrections.

The CubeMX-generated diskio integration layer was found to be unreliable
(10-second hangs on card removal, use of FreeRTOS queues from ISR context
without proper semaphore coordination) and was discarded.  Everything below
this line describes the replacement implementation.

---

## Files

| File | Role |
|---|---|
| `Core/Inc/task_sdcard.h` | Public API: `task_sdcard_init`, `task_sdcard_push_frame`, `ms_sd_card`, `sdcard_on_hal_error` |
| `Core/Src/task_sdcard.c` | State machine, card-detect debounce, DMA semaphore, FatFS lifecycle, IDLE write loop |
| `FATFS/Target/sd_diskio.c` | FatFS disk I/O driver: `disk_initialize/status/read/write/ioctl`, BSP completion callbacks |
| `FATFS/Target/fatfs_platform.c` | `BSP_PlatformIsDetected` — overridden in USER CODE to return the debounced flag |

---

## Hardware

- **SDMMC2** peripheral, 4-bit wide bus, clock ~24 MHz (high-speed mode after init).
- **Card detect**: GPIOG pin 13, active-low (`GPIO_PIN_RESET` = card present).
- **DMA**: two DMA streams pre-configured by CubeMX for SDMMC2 TX and RX.
  `HAL_SD_ReadBlocks_DMA` / `HAL_SD_WriteBlocks_DMA` use these streams;
  the polling equivalents (`HAL_SD_ReadBlocks` / `HAL_SD_WriteBlocks`) must
  **not** be used — see [DMA is mandatory](#dma-is-mandatory).

---

## State Machine

```
                insert (20 ms debounce fires)
    ┌───────────────────────────────────────────────────────────┐
    │                                                           ▼
┌────────┐  notification  ┌──────┐  Init OK  ┌──────────┐  mount+open OK  ┌──────┐
│ ABSENT │───────────────▶│ INIT │──────────▶│ MOUNTING │────────────────▶│ IDLE │
│        │◀───────────────│      │           │          │                 │      │
└────────┘   error/eject  └──────┘           └──────────┘                └──────┘
    ▲                                              │ error                    │
    └──────────────────────────────────────────────┴──── eject or write error ┘
```

All transitions to **ABSENT** call `sd_teardown()`, which:
1. `f_close(&fp)` — flushes and closes the open file (if any).
2. `f_mount(NULL, "", 0)` — unmounts the FatFS volume.
3. `HAL_SD_DeInit(&hsd2)` — de-initialises the HAL handle.
4. `__HAL_RCC_SDMMC2_FORCE_RESET()` + 2 ms + `__HAL_RCC_SDMMC2_RELEASE_RESET()` — resets the peripheral hardware so the next init starts clean.

---

## Card Detect Debounce

The card detect pin is polled from the **TIM6 1 ms tick ISR** (`ms_sd_card()`,
called from `HAL_TIM_PeriodElapsedCallback` in `main.c` USER CODE).

- **Insert**: 20 consecutive milliseconds of `GPIO_PIN_RESET` (active-low)
  are required before `sd_card_present` is set to 1 and the task is notified.
  This debounces contact bounce on insertion without masking a genuine fast eject.
- **Eject**: `GPIO_PIN_SET` clears `sd_card_present` to 0 and notifies the
  task **immediately** (no debounce).  A false eject is safer than a missed
  eject — the worst outcome is an unnecessary teardown and remount; a missed
  eject risks FatFS operating on a card that is no longer physically present.

```
 GPIO (active-low)   ─────┐           ┌────────────────────
                           └─┐         │
 sd_detect_count (0–20)      └─────────┘ 0  1  2 ··· 20  (saturates)
 sd_card_present              0                          1
```

The `BSP_PlatformIsDetected()` function in `fatfs_platform.c` is overridden
(in USER CODE BEGIN 1) to return the debounced `sd_card_present` flag rather
than reading the GPIO directly.  All FatFS internal callers (e.g. `f_mount`
disk-check paths) therefore see the debounced result automatically.

---

## INIT State

```c
__HAL_RCC_SDMMC2_FORCE_RESET();
HAL_Delay(2);
__HAL_RCC_SDMMC2_RELEASE_RESET();
HAL_SD_DeInit(&hsd2);
HAL_SD_Init(&hsd2);
/* Poll until card confirms TRANSFER state (CMD13): */
while (BSP_SD_GetCardState() != SD_TRANSFER_OK) { ... }
```

The RCC force-reset ensures a clean peripheral state on every insertion,
even if the previous session ended in an error.  `HAL_SD_DeInit` before
`HAL_SD_Init` causes the HAL to call `HAL_SD_MspDeInit` followed by
`HAL_SD_MspInit`, which reconfigures GPIO, DMA streams, and NVIC from scratch.

When the card is already powered at MCU boot (i.e. was inserted before
power-on), `HAL_SD_Init` can complete in ~21 ms rather than the usual
~180 ms — the card is already stable so ACMD41 returns immediately.  A brief
`BSP_SD_GetCardState()` poll (CMD13, up to 1 s) after `HAL_SD_Init` confirms
TRANSFER state before handing off to FatFS, covering this fast-init path.

---

## MOUNTING State

```c
FRESULT r = f_mount(&fs, "", 1);  /* force immediate mount */
```

`f_mount` with `force = 1` calls `disk_initialize` then reads the boot sector
and FAT.  If it returns `FR_OK`, `open_next_file()` scans for the first unused
`RTCM####.BIN` filename and opens it with `FA_CREATE_NEW | FA_WRITE`.  Both
operations require `disk_read` and `disk_write` to be working — see
[DMA is mandatory](#dma-is-mandatory).

On any FatFS error the state machine calls `sd_teardown()` and returns to
**ABSENT**.  The task then blocks on `ulTaskNotifyTake`, waiting for the next
card-detect notification.  The user must physically eject and reinsert to
trigger a new mount attempt.

---

## IDLE State

The IDLE loop drains the write queue and checks for eject on each 200 ms
timeout:

```c
while (next == SD_IDLE) {
    BaseType_t got = xQueueReceive(sd_q, &sd_recv_frame, pdMS_TO_TICKS(200));

    if (!sd_card_present) {
        /* eject detected — sd_teardown() + ABSENT */
    }
    if (got == pdTRUE) {
        f_write(&fp, sd_recv_frame.data, sd_recv_frame.len, &bw);
        f_sync(&fp);   /* flush FAT + directory entry after every frame */
    }
}
```

`f_sync` after every frame ensures the file is recoverable from the card if
power is removed without a clean unmount.  The overhead is one extra
`disk_write` per frame (FAT sector + directory entry), typically < 5 ms.

The write queue (`sd_q`) holds 3 slots of `sd_frame_t` (1032 bytes of RTCM3
data + 2-byte length = 1034 bytes each, ~3 KB total).  Frames are pushed by
`task_sdcard_push_frame()` from the OTA task; the queue is non-blocking on
push — a frame is silently dropped if all 3 slots are occupied.

---

## DMA Transfer Model

All disk reads and writes use DMA.  The flow for a single sector:

```
task context (sdcard_task)              ISR context (DMA / SDMMC IRQ)
────────────────────────────────        ──────────────────────────────────
sd_xfer_error = 0
HAL_SD_ReadBlocks_DMA(...)   ──starts─▶  ... DMA transfer ...
xSemaphoreTake(sd_xfer_done, 5 s)            │
        (task blocks, yields CPU)             │ transfer complete
                                       HAL_SD_RxCpltCallback
                                       → BSP_SD_ReadCpltCallback
                                       → xSemaphoreGiveFromISR(sd_xfer_done)
xSemaphoreTake returns ◀──────────────────────┘
check sd_xfer_error
```

For writes, after the semaphore is taken, `BSP_SD_GetCardState()` is polled
until the card returns to TRANSFER state — the card enters PRG (programming)
state while writing its internal flash and must not receive another command
until it exits.

HAL error callback (`HAL_SD_ErrorCallback`, wired in `main.c` USER CODE and
forwarded to `sdcard_on_hal_error()` in `task_sdcard.c`) also gives the
semaphore and sets `sd_xfer_error = 1`, ensuring the blocked `disk_read` /
`disk_write` is always unblocked even on error or card removal mid-transfer.

```c
/* task_sdcard.c */
SemaphoreHandle_t sd_xfer_done;   /* created in task_sdcard_init() */
volatile uint8_t  sd_xfer_error;  /* set from ISR, cleared before each transfer */
```

Both are `extern`'d into `sd_diskio.c` and the BSP callbacks.

---

## DMA is Mandatory

`HAL_SD_ReadBlocks` and `HAL_SD_WriteBlocks` (polling mode) must **not** be
used in this firmware.  The polling functions service the SDMMC RXFIFO /
TXFIFO in a tight CPU loop.  If the FreeRTOS scheduler preempts `sdcard_task`
mid-loop (e.g. `task_logger`, which runs at a higher priority, becomes ready),
the FIFO overflows or underdrains.  The SDMMC hardware asserts
`SDMMC_FLAG_RXOVERR` / `TXUNDERR`, and the HAL returns `HAL_ERROR` immediately
— not after a timeout.  In practice this caused every `disk_write` to fail,
meaning `f_open` could never create a file regardless of how many filenames
were tried.

The DMA path has no CPU involvement in the data transfer, so task preemption
has no effect.

---

## Cortex-M7 D-Cache Maintenance

The F767ZI's Cortex-M7 has a 16 KB L1 D-cache.  DMA transfers bypass the
cache (DMA reads/writes physical memory directly), so explicit maintenance is
required to keep the cache coherent:

**Before a DMA read** (SD card → memory):
```c
SCB_InvalidateDCache_by_Addr((uint32_t *)rdst, BLOCKSIZE);
```
Marks the destination cache lines as invalid so the CPU reads fresh data from
memory (not from a stale prefetch) after the transfer completes.

**Before a DMA write** (memory → SD card):
```c
SCB_CleanDCache_by_Addr((uint32_t *)wbuf, BLOCKSIZE);
```
Writes back any dirty cache lines to memory so the DMA sees the data the CPU
actually wrote.

Both functions require their pointer argument to be **32-byte aligned**
(one cache line = 32 bytes on Cortex-M7) and the length to be a multiple of
32 bytes.  `BLOCKSIZE = 512` is exactly 16 cache lines.  Any DMA buffer that
is not 32-byte aligned is bounced through a module-static scratch buffer:

```c
static uint8_t scratch[BLOCKSIZE] __attribute__((aligned(32)));
```

FatFS's internal sector buffer (`fs->win`, 512 bytes) is part of a statically-
allocated `FATFS` struct and may not be 32-byte aligned; the scratch path
handles this transparently.

---

## HAL Callback Routing

Per project convention, all HAL weak-override callbacks live in `main.c`
USER CODE sections and forward to named subfunctions:

```c
/* main.c — USER CODE BEGIN Callback 1 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        ms_sd_card();   /* card-detect debounce, 1 ms tick */
    }
}

/* main.c — USER CODE BEGIN 4 */
void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
    sdcard_on_hal_error(hsd);
}
```

The BSP completion callbacks (`BSP_SD_WriteCpltCallback`, `BSP_SD_ReadCpltCallback`,
`BSP_SD_AbortCallback`) are strong overrides of the weak stubs in
`bsp_driver_sd.c` and live in `sd_diskio.c` — they are SD-diskio-specific
and do not belong in `main.c`.

---

## Integration

`task_ota.c` wires the SD card into the RTCM3 receive path via the
`on_rtcm3_frame` callback:

```c
static void on_rtcm3_frame(const uint8_t *frame, uint16_t len)
{
    ota_tx_push_frame(&ota_tx, frame, len);   /* transmit over LoRa */
    task_sdcard_push_frame(frame, len);        /* log to SD card     */
}
```

`task_sdcard_push_frame` copies the frame into the write queue and returns
immediately.  The caller (OTA task) is never stalled.

Initialisation order in `main.c`:
1. `MX_FATFS_Init()` — registers the SD driver with FatFS (assigns logical
   drive `""` / `"0:"`).
2. `task_sdcard_init()` — creates `sd_xfer_done` semaphore, write queue, and
   spawns `sdcard_task` (stack 1024 words, priority `tskIDLE_PRIORITY + 1`).
3. `task_ota_init()` — spawns OTA task; RTCM3 UART traffic begins.
