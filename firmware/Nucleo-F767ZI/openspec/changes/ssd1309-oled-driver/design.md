## Context

The sandbox has I2C1 (PB8/SCL, PB9/SDA, internal pull-ups, 400 kHz, see
project `CLAUDE.md`) brought up and idle. A scaffold for an SSD1309 driver
exists (`Core/Inc/ssd1309.h`, `Core/Src/ssd1309.c`, commit #46) but
`ssd1309_init()` currently only fills in a struct — no I2C traffic is sent.
The SSD1309 datasheet (`docs/datasheets/SSD1309.pdf`) defines the I2C
interface (control-byte framing: `0x00` prefix for command stream, `0x40`
prefix for data stream — Section 8.1.5) and the fundamental/addressing
command set (Section 9-10).

This change is the first real exercise of I2C1 and the first OLED bring-up
on this board. The goal is to get to "panel lights up" as cheaply as
possible so wiring/address/command-sequence problems are caught before any
framebuffer/graphics layer is built.

## Goals / Non-Goals

**Goals:**
- Send a correct SSD1309 power-on init command sequence over I2C1.
- Light the entire 128x64 panel (all pixels on) as a wiring/bring-up check.
- Fix the scaffold's `ssd1309_t` to hold a usable I2C handle reference.
- Hook the bring-up call into `app_init()`/`app_loop()` per the sandbox's
  cooperative-scheduler pattern.

**Non-Goals:**
- No framebuffer, drawing primitives, or text/font rendering.
- No DMA/interrupt-driven I2C transfers — blocking `HAL_I2C_*` calls only.
- No display reset (RES#) GPIO control — the I2C OLED module is assumed to
  tie RES# to VCC via the module's own reset circuit (common on cheap
  I2C-only SSD1309/SSD1306 breakout boards). If the panel doesn't come up,
  this is the first thing to check on the bench.
- No runtime configuration of panel size/orientation beyond the existing
  `height`/`width` fields — this driver targets the 128x64 panel on hand.

## Decisions

- **`ssd1309_t` holds `I2C_HandleTypeDef *port`, not a copy.** The scaffold
  currently does `I2C_HandleTypeDef port` (pass-by-value struct copy), which
  works but is wasteful and diverges from how `&hi2c1` is used everywhere
  else (e.g. `HAL_UART_Transmit(&huart3, ...)` in `app.c`). Change the field
  to `I2C_HandleTypeDef *port` and update `ssd1309_init()`'s signature to
  take `I2C_HandleTypeDef *in_port`.

- **I2C address stored as 7-bit, shifted to 8-bit at the HAL call site.**
  `ssd1309_t.address` keeps the 7-bit address (e.g. `0x3C`) as documented on
  the module/datasheet; `ssd1309_init()`/internal helpers left-shift by 1
  before passing to `HAL_I2C_Mem_Write` (which expects the 8-bit
  R/W-bit-inclusive address). This matches how the address is usually
  silkscreened/documented for these modules and keeps the public API in the
  units a user would expect.

- **Command/data writes via `HAL_I2C_Mem_Write` using the control byte as
  the "register address".** Per SSD1309 I2C framing (Section 8.1.5), every
  transfer is `[slave addr][control byte][payload...]`. `HAL_I2C_Mem_Write(
  hi2c, addr<<1, control_byte, I2C_MEMADD_SIZE_8BIT, data, len, timeout)`
  maps directly onto this: `control_byte = 0x00` for a stream of command
  bytes, `0x40` for a stream of data (GDDRAM) bytes. This avoids hand-rolling
  `HAL_I2C_Master_Transmit` with a manually-built buffer for the common case.
  A short fixed timeout (e.g. 100 ms, matching the existing
  `HAL_UART_Transmit` timeout convention in `app.c`) is used throughout.

- **Init command sequence (static `const uint8_t[]`, sent as one
  `HAL_I2C_Mem_Write` with control byte `0x00`):**
  `0xAE` (display off) →
  `0xD5 0x80` (clock divide/osc freq) →
  `0xA8 0x3F` (multiplex ratio = 64, for height 64) →
  `0xD3 0x00` (display offset = 0) →
  `0x40` (display start line = 0) →
  `0xA1` (segment re-map, column 127 → SEG0) →
  `0xC8` (COM output scan direction, remapped) →
  `0xDA 0x12` (COM pins hardware config) →
  `0x81 0x8F` (contrast) →
  `0xD9 0xF1` (pre-charge period) →
  `0xDB 0x40` (VCOMH deselect level) →
  `0xA4` (entire display on: resume to RAM content) →
  `0xA6` (normal, non-inverted display) →
  `0x2E` (deactivate scroll) →
  `0xAF` (display on).
  This is the standard SSD1309/SSD1306-family sequence; SSD1309 has no
  internal charge pump (`0x8D`), so that command is omitted (panel is
  externally VCC-driven).

- **Test pattern via page-addressing mode, all-bytes-`0xFF`.** After init,
  set memory addressing mode to page addressing (`0x20 0x02`), then for each
  of the 8 pages (`0xB0`..`0xB7`) reset the column pointer to 0
  (`0x00`, `0x10`) and write 128 bytes of `0xFF` with control byte `0x40`.
  This lights every pixel — a clear, unambiguous bring-up indicator visible
  even if framebuffer addressing logic is later wrong.

- **API surface for this change:** keep `ssd1309_init()` for struct setup
  only (no I2C traffic — matches its current name/contract), and add a new
  `HAL_StatusTypeDef ssd1309_bringup(ssd1309_t *p)` that performs the init
  sequence + test pattern + display-on. Keeping `_init` side-effect-free
  avoids surprising callers and leaves room for a future `ssd1309_clear()`
  / framebuffer init to reuse the same struct without re-running the full
  power-on sequence.

- **Integration point: `app_init()`.** Call `ssd1309_init(&oled, &hi2c1,
  0x3C, -1, -1)` then `ssd1309_bringup(&oled)` once from `app_init()`. The
  `oled` struct can be a local-static or file-static in `app.c` for now (no
  shared display state needed yet). `app_loop()` is unchanged otherwise.

## Risks / Trade-offs

- **I2C address may be `0x3D` instead of `0x3C`** depending on the module's
  SA0 strap → Mitigation: document `0x3C` as the assumed default in code
  comments/header doc, easy one-line change on the bench if `HAL_I2C_*`
  returns `HAL_ERROR`/NACK.
- **No reset control** → if the panel doesn't initialize reliably after
  power-up, the lack of a controlled RES# pulse could be the cause →
  Mitigation: documented as the first thing to check (Non-Goals); adding a
  GPIO-driven reset is a small follow-up if needed.
- **Single large blocking I2C write for the init sequence** (~26 bytes) and
  8x128-byte writes for the test pattern block `app_init()` for a short but
  non-zero time → acceptable for one-time bring-up at boot; revisit if a
  later change moves display updates into the periodic loop.
- **Magic command bytes with no named constants** could be hard to
  maintain → Mitigation: define `#define`/enum constants for the SSD1309
  command opcodes used, with comments referencing the datasheet section,
  rather than bare hex in the sequence array.

## Open Questions

- Confirm on the bench: I2C address `0x3C` vs `0x3D`, and whether the
  all-on test pattern displays correctly (validates segment re-map / COM
  scan direction choices above, which are mirrored/orientation-dependent
  but invisible on an all-on pattern — full visual confirmation will need a
  follow-up framebuffer change with a non-symmetric pattern).
