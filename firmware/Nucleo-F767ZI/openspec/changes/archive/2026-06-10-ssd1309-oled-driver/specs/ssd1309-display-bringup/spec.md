## ADDED Requirements

### Requirement: SSD1309 handle initialization
The driver SHALL provide `ssd1309_init()` to populate an `ssd1309_t` handle
with an I2C peripheral handle pointer, 7-bit I2C address, and panel
dimensions, without performing any I2C bus transactions.

#### Scenario: Default dimensions
- **WHEN** `ssd1309_init()` is called with `in_height` and `in_width` both
  `-1`
- **THEN** the resulting handle has `height == 64` and `width == 128`

#### Scenario: Explicit dimensions and address stored
- **WHEN** `ssd1309_init()` is called with an `I2C_HandleTypeDef *`, a 7-bit
  I2C address, and explicit positive `height`/`width` values
- **THEN** the resulting handle stores that I2C handle pointer, that 7-bit
  address (unshifted), and the given `height`/`width` values
- **AND** no I2C transaction is performed as part of this call

### Requirement: Power-on initialization sequence
The driver SHALL provide `ssd1309_bringup()`, which sends the SSD1309
power-on command sequence over I2C using the handle's stored I2C peripheral
and 7-bit address (shifted left by 1 for the HAL call), configuring the
display for normal (non-inverted) operation at the handle's panel
dimensions, and leaves the display powered on (`0xAF`).

#### Scenario: Successful bring-up returns HAL_OK
- **WHEN** `ssd1309_bringup()` is called on a handle initialized via
  `ssd1309_init()` and the panel acknowledges all I2C transfers
- **THEN** the function sends the documented init command sequence
  (display off; clock divide; multiplex ratio derived from `height`;
  display offset 0; start line 0; segment re-map; COM scan direction;
  COM pins config; contrast; pre-charge; VCOMH; entire-display-on/resume;
  normal display; deactivate scroll; display on)
- **AND** returns `HAL_OK`

#### Scenario: I2C failure is propagated
- **WHEN** any I2C transfer during `ssd1309_bringup()` fails (NACK, bus
  error, or timeout)
- **THEN** `ssd1309_bringup()` returns the corresponding non-`HAL_OK`
  `HAL_StatusTypeDef` from the failing `HAL_I2C_*` call without sending
  further commands

### Requirement: All-on test pattern
As part of `ssd1309_bringup()`, after the init command sequence the driver
SHALL write `0xFF` to every byte of GDDRAM for the handle's panel
dimensions using page-addressing mode, so that every pixel is lit.

#### Scenario: Full-panel test pattern written
- **WHEN** `ssd1309_bringup()` runs successfully on a 128x64 handle
- **THEN** for each of the 8 pages (`0xB0`-`0xB7`) the driver resets the
  column address to 0 and writes 128 bytes of `0xFF` to GDDRAM with the
  I2C data control byte (`0x40`)
- **AND** after this, every pixel on the panel is in the "on" state
