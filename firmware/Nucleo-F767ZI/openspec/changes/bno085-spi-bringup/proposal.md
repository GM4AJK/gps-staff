## Why

The BNO085 9-DOF IMU is now wired to the Nucleo-F767ZI sandbox over SPI1
(SCK/MISO/MOSI on PA5/PA6/PA7, CS on PD14, RST on PD15, active-low INT on
PF12, with P0/P1 bridged high for SPI mode). Before building out the full
SHTP sensor-report driver, we need a minimal bring-up that resets the chip,
waits for its post-reset SHTP advertisement packet over SPI, and confirms
the wiring/SPI configuration is correct.

## What Changes

- Add a `bno085_t` handle type and `bno085_init()` to populate it with an
  SPI peripheral handle pointer and the CS/RST/INT GPIO ports/pins, without
  performing any SPI transactions.
- Add `bno085_bringup()`, which:
  - Drives `RST` low then high (with delays) to reset the chip.
  - Waits for `INT` to go low (with a timeout), indicating the chip has data
    ready.
  - Performs a single SPI read of the SHTP header + post-reset advertisement
    payload into a buffer in the handle.
- Add a demo in `app_init()` that runs the bring-up and reports success /
  failure (and the received byte count) over USART3.

## Capabilities

### New Capabilities
- `bno085-spi-bringup`: BNO085 SPI handle initialization, hardware reset
  sequence, and reading the post-reset SHTP advertisement packet over SPI to
  confirm wiring/configuration.

## Impact

- New files `Core/Src/bno085.c` / `Core/Inc/bno085.h`.
- `Core/Src/app.c`: demo bring-up call and UART status report.
