## Why

The Nucleo-F767ZI sandbox has I2C1 wired up and a stub `ssd1309_init()` already
scaffolded, but the SSD1309 OLED panel has never been brought up. Before any
framebuffer/graphics work is built on top of it, the driver needs to send the
correct power-on/init command sequence over I2C and prove the panel responds
(by lighting the whole display), confirming the wiring, I2C address, and
command sequence are correct.

## What Changes

- Implement the SSD1309 power-on initialization command sequence (display
  off, clock/multiplex/offset/charge-pump/contrast/COM-pins/etc. setup,
  segment re-map, COM scan direction, normal display mode) sent over I2C1
  using blocking HAL calls.
- Send a simple static test pattern to GDDRAM (e.g. all pixels on, or a
  checkerboard) using page-addressing mode writes, then turn the display on.
- Wire a minimal call into `app_init()` / `app_loop()` of the sandbox `app.c`
  so the panel lights up on boot, as a bring-up smoke test.
- Extend `ssd1309_t` / `ssd1309_init()` only as needed to support init +
  test-pattern (e.g. storing the `I2C_HandleTypeDef *` and I2C address
  correctly, fixing the existing pass-by-value handle if needed).

Out of scope for this change: a general framebuffer API, drawing primitives,
text/font rendering, and DMA/non-blocking transfers — these are left for a
follow-up change once bring-up is confirmed working on the bench.

## Capabilities

### New Capabilities
- `ssd1309-display-bringup`: SSD1309 OLED panel I2C initialization and
  static test-pattern display, used to validate wiring and command sequence
  on the Nucleo-F767ZI sandbox.

### Modified Capabilities
(none)

## Impact

- `Core/Inc/ssd1309.h`, `Core/Src/ssd1309.c` — implement init command
  sequence and test-pattern write; likely change `ssd1309_t` to hold an
  `I2C_HandleTypeDef *` instead of a copied struct.
- `Core/Src/app.c` — call the new init/test-pattern functions from
  `app_init()`/`app_loop()` for bench bring-up.
- I2C1 bus (PB8/PB9, 400 kHz) — first real traffic on this bus.
