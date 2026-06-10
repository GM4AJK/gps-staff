## MODIFIED Requirements

### Requirement: Wait for data-ready and read SHTP packet
Immediately before releasing reset, `bno085_bringup()` SHALL discard any
previously-latched `BNO085_INT` edge flag (`flag_get_BNO085_INT()`).
After releasing reset, `bno085_bringup()` SHALL wait for `INT` to become
active, with a timeout of `BNO085_INT_TIMEOUT_MS`. `INT` is considered
active as soon as either the `BNO085_INT` edge flag is set (a falling
edge was latched by `HAL_GPIO_EXTI_Callback()` at any point since it was
discarded) or the `INT` GPIO pin currently reads low - whichever is
observed first. If `INT` becomes active within the timeout, the driver
SHALL assert `CS` low, perform a single `BNO085_BRINGUP_BUF_SIZE`-byte
full-duplex SPI transfer (transmitting all-zero bytes), then release
`CS` high. The driver SHALL parse the first 4 received bytes as the SHTP
header: a 16-bit length (LSB first, with the continuation bit masked
off) into `shtp_length`, followed by one byte each into `shtp_channel`
and `shtp_sequence`. The full received buffer SHALL be stored in the
handle.

#### Scenario: Successful read populates the handle and returns HAL_OK
- **WHEN** `bno085_bringup()` is called, `INT` becomes active (edge
  latched or level low) within `BNO085_INT_TIMEOUT_MS` of releasing
  `RST`, and the SPI transfer succeeds
- **THEN** `bno085_bringup()` returns `HAL_OK`
- **AND** the handle's `shtp_length`, `shtp_channel`, and `shtp_sequence`
  fields reflect the first 4 bytes received
- **AND** the handle's raw receive buffer contains all
  `BNO085_BRINGUP_BUF_SIZE` bytes received over SPI

#### Scenario: A falling edge latched before the level-poll loop starts is not missed
- **WHEN** `bno085_bringup()` is called, the `BNO085_INT` edge flag is
  discarded immediately before `RST` is released, `INT` briefly pulses
  low and back high (latching the flag via
  `HAL_GPIO_EXTI_Callback()`) before the level-poll loop begins, and the
  SPI transfer succeeds
- **THEN** `bno085_bringup()` returns `HAL_OK` without waiting for
  `BNO085_INT_TIMEOUT_MS`

#### Scenario: INT timeout returns an error without an SPI transaction
- **WHEN** `bno085_bringup()` is called and `INT` does not become active
  (no edge latched and the level never reads low) within
  `BNO085_INT_TIMEOUT_MS` of releasing `RST`
- **THEN** `bno085_bringup()` returns `HAL_TIMEOUT`
- **AND** no SPI transfer is performed
- **AND** `CS` remains high (deasserted)

#### Scenario: SPI failure during the read is propagated
- **WHEN** `INT` becomes active within the timeout, but the
  `HAL_SPI_TransmitReceive()` call fails (error or timeout)
- **THEN** `bno085_bringup()` returns the corresponding non-`HAL_OK`
  `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
