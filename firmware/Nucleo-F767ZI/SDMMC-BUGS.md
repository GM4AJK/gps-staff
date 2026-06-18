# STM32 SDMMC / FatFS Known Bugs and Fixes

This file documents root causes and fixes for CubeMX-generated SDMMC and
FatFS bugs discovered during development on the F767ZI sandbox board.
Each entry is tagged with a short ID so it can be cross-referenced from
source comments (e.g. `/* BUG-SDMMC-001 */`).

---

## BUG-SDMMC-001 — Boot crash when no SD card is inserted

**Symptom**
The board hits `Error_Handler()` immediately on every boot if no SD card is
in the slot.

**Root cause**
CubeMX generates `MX_SDMMC2_SD_Init()` which calls `HAL_SD_Init()` uncondi-
tionally. `HAL_SD_Init()` runs the full SD card power-up and enumeration
sequence (CMD0 → CMD8 → ACMD41/CMD41 → CMD2 → CMD3). If no card is present,
no response is received, the HAL returns `HAL_ERROR`, and the generated code
calls `Error_Handler()`.

**Fix**
Add a Card Detect (CD) pin check in `USER CODE BEGIN SDMMC2_Init 0` (the
earliest USER CODE slot, executed before any field assignments or HAL calls)
and return early when the CD pin reads `GPIO_PIN_SET` (no card; pull-up
active):

```c
/* USER CODE BEGIN SDMMC2_Init 0 */
if (HAL_GPIO_ReadPin(SDMMC_CD_GPIO_Port, SDMMC_CD_Pin) == GPIO_PIN_SET) {
    return;
}
/* USER CODE END SDMMC2_Init 0 */
```

Application code is then responsible for calling `HAL_SD_Init()` (and
`HAL_SD_ConfigWideBusOperation()`) when a card insertion is detected via a
CD pin edge or a polling loop.

**Affected section**
`Core/Src/main.c` → `MX_SDMMC2_SD_Init()`

---

## BUG-SDMMC-002 — Card does not remount after eject and reinsertion

**Symptom**
After a card is physically removed and reinserted, `f_mount()` fails or the
filesystem is not accessible. The problem does not occur on a fresh power
cycle.

**Root cause**
*(Under investigation — to be confirmed with FatFS middleware enabled.)*
Likely candidates from prior experience with this hardware:
- `HAL_SD_Init()` is not re-called after insertion; the HAL handle state is
  left in `HAL_SD_STATE_READY` referencing the old card's RCA/CID.
- The SDMMC peripheral is not fully reset between eject and re-init; residual
  clock gating or CMD/DAT line state confuses the new card's power-up.
- FatFS `f_mount()` with `opt=1` (immediate mount) does not trigger a fresh
  `HAL_SD_Init()` if the HAL handle is already initialised.

**Fix**
*(Pending — to be documented once confirmed.)*

**Affected section**
TBD — likely `MX_SDMMC2_SD_Init()`, application card-detect handler, and/or
FatFS `diskio.c`.

---

## BUG-SDMMC-003 — HAL_SD_ERROR_RX_OVERRUN in polling mode (ClockDiv=0)

**Symptom**
`f_mount()` returns `FR_DISK_ERR (1)`. Adding `hsd2.ErrorCode` logging shows
`0x00000020 = HAL_SD_ERROR_RX_OVERRUN` — the SDMMC RX FIFO overflowed during
the first sector read.

**Root cause**
STM32F7 errata (ES0334): the SDMMC hardware flow-control feature is not
correctly implemented and must not be used. Without hardware flow control, the
CPU must drain the RX FIFO fast enough by polling to prevent overflow.
At ClockDiv=0 the SDMMC_CK is 24 MHz; in 4-bit mode data arrives at 12 MB/s,
filling the 128-byte FIFO in ~10.7 μs. Any interrupt latency or pipeline stall
can cause an overrun before the polling loop drains the FIFO.

**Fix**
Superseded by BUG-SDMMC-004: switching to 1-bit mode (which skips
`HAL_SD_ConfigWideBusOperation`) removes the 4-bit FIFO pressure entirely.
In 1-bit mode the FIFO fills 4× slower for the same SDMMC_CK, so ClockDiv=0
(24 MHz, ~3 MB/s) is safe in polling mode without any clock reduction needed.

If 4-bit mode is ever restored (once wiring is fixed), keep ClockDiv ≥ 6
(SDMMC_CK ≤ 8 MHz) for polling safety, or switch to the DMA template.

**DMA note (if 4-bit + full speed is ever needed)**
Enable "Use dma template" in CubeMX FatFS settings and configure DMA2 Stream 0
Channel 11 for SDMMC2. DMA removes the FIFO-drain constraint entirely and
allows ClockDiv=0 in 4-bit mode. Note: F7 D-Cache is enabled — add
`SCB_InvalidateDCache_by_Addr()` after DMA reads and
`SCB_CleanDCache_by_Addr()` before DMA writes in `sd_diskio.c` USER CODE
sections.

**Affected section**
`Core/Src/main.c` → `MX_SDMMC2_SD_Init()`

---

## BUG-SDMMC-004 — DATA_CRC_FAIL in 4-bit mode; D1/D2/D3 wiring fault on SD BOB

**Symptom**
After fixing BUG-SDMMC-003 (ClockDiv), `f_mount()` still returns
`FR_DISK_ERR (1)` with `hsd2.ErrorCode = 0x00000002 (HAL_SD_ERROR_DATA_CRC_FAIL)`.
Switching to 1-bit mode (override `BSP_SD_Init()` to skip
`HAL_SD_ConfigWideBusOperation()`) makes everything work: mount, write, read
all succeed.

**Root cause**
4-bit data CRC is calculated across all four DAT lines simultaneously by the
SDMMC hardware. A floating or disconnected DAT1, DAT2, or DAT3 line causes
corrupted data on that lane, failing the CRC on every block read. CMD/response
(which run on the CMD line only, in 1-bit mode during init) are unaffected, so
`HAL_SD_Init()` and `HAL_SD_ConfigWideBusOperation()` both return `HAL_OK`
— the fault is invisible until the first actual data transfer in 4-bit mode.

Affected pins: PG10 (SDMMC2_D1), PG11 (SDMMC2_D2), PG12 (SDMMC2_D3).
Check wiring from these Morpho header pins to the SD breakout board.

**Fix**
Inspect and re-seat the D1/D2/D3 wires on the SD breakout board. Once fixed,
remove the `BSP_SD_Init()` override in `Core/Src/app.c` to restore 4-bit mode.

As a workaround until the wiring is fixed (or if 1-bit mode is sufficient for
the application — ~570 KB/s at ClockDiv=40 is far more than needed for RTCM
logging), the override in `app.c` forces 1-bit mode:

```c
uint8_t BSP_SD_Init(void)  /* strong override of __weak in bsp_driver_sd.c */
{
    extern SD_HandleTypeDef hsd2;
    if (BSP_SD_IsDetected() != SD_PRESENT)
        return MSD_ERROR_SD_NOT_PRESENT;
    return (HAL_SD_Init(&hsd2) == HAL_OK) ? MSD_OK : MSD_ERROR;
}
```

**Affected section**
`FATFS/Target/bsp_driver_sd.c` → `BSP_SD_Init()` (override in
`Core/Src/app.c`)

---
