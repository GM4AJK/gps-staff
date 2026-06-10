# Nucleo-F767ZI Sandbox

## What this is

A CubeMX-generated STM32F767ZI Nucleo project used as a bench "play pen"
for developing and testing firmware modules in isolation before porting
the working code into `rtk-base`/`rtk-rover` (the actual gps-staff
boards). Pin assignments and exact peripherals here are specific to this
sandbox board and will differ slightly from the real hardware.

## Hardware setup (this specific board)

- The on-board ST-LINK has been physically cut away (it's in use
  programming other targets). Programming/debugging is done via an
  **external ST-LINK V3** wired to the Morpho headers — see
  `README.md` for the full pinout table.
- Powered from a bench PSU via V_IN (CN8 pin 15), 7.0 V / 500 mA — see
  `README.md` for jumper settings (JP3 5-6 bridged, JP1 off).
- **No HSE available**: the ST-LINK MCO clock source is gone (cut) and
  no X3 crystal is fitted. The system clock runs from **HSI only**
  (`SystemClock_Config`: HSI → PLL, PLLM=8/PLLN=216/PLLP=2/PLLQ=9 →
  216 MHz SYSCLK).
- **USB_OTG_FS and ETH are disabled** — both peripherals require HSE for
  an accurate clock, so they can't be used while running on HSI. Don't
  re-enable them without first fitting an external 8 MHz crystal.
  Note: `MX_GPIO_Init()` still has some leftover inert RMII pin config
  from before ETH was removed — harmless, pending a later pin audit.

## Current peripherals

- **USART3** — debug/VCP UART, routed to the external ST-LINK V3's VCOM
  port. TX = PB10, RX = PD9 (TX was moved from PD8 to PB10 for easier
  bench wiring).
- **I2C1** — PB8 (SCL) / PB9 (SDA), internal pull-ups, Fast mode
  (400 kHz, `Timing=0x6000030D`). Added in preparation for the first I2C
  sensor/device library to be developed here.
- **LD1 / LD2** — onboard LEDs, currently used as a simple alive
  indicator.

## Application structure (`Core/Src/app.c`)

Cooperative scheduler pattern shared with `rtk-base`/`rtk-rover`:

- `app_1ms()` — called from a 1ms timer tick; increments counters and
  sets `flag_set_10MS()` / `flag_set_100MS()` / `flag_set_1000MS()` via
  the `flags` module (`flag_set_*` / `flag_get_*` / `flag_peek_*`,
  defined in `Core/Inc/flags.h`).
- `app_init()` — one-time init, currently a stub (`// TDo`).
- `app_loop()` — main loop (currently: blinks LD1/LD2 and sends a
  `"loop %lu\r\n"` heartbeat over USART3 every 500ms via
  `HAL_Delay`/`HAL_UART_Transmit`).

When adding a new module to test here, follow this pattern: init in
`app_init()`, periodic work in `app_loop()` gated on the flag helpers
where appropriate, rather than ad-hoc delays.

## Workflow reminders

- Same repo, same rules as the rest of gps-staff: **PR-only on `main`**
  — feature branch, commit, push, `gh pr create`, wait for the user to
  say "merge and update".
- CubeMX `.ioc` regenerations should be reviewed and committed alongside
  the corresponding `main.c`/`main.h` changes they produce.
