## ADDED Requirements

### Requirement: BNO085 handle initialization
The driver SHALL provide `bno085_init()` to populate a `bno085_t` handle with
an SPI peripheral handle pointer and the GPIO ports/pins for `CS`, `RST`, and
`INT`, without performing any SPI transactions or GPIO writes.

#### Scenario: Handle populated without bus activity
- **WHEN** `bno085_init()` is called with an `SPI_HandleTypeDef *` and the
  `CS`/`RST`/`INT` GPIO ports and pins
- **THEN** the resulting handle stores the SPI handle pointer and the
  `CS`/`RST`/`INT` GPIO ports/pins
- **AND** no SPI transaction or GPIO write is performed as part of this call

### Requirement: Hardware reset sequence
The driver SHALL provide `bno085_bringup()`, which begins by driving `RST`
low for `BNO085_RESET_PULSE_MS`, then driving `RST` high.

#### Scenario: Reset pulse is issued
- **WHEN** `bno085_bringup()` is called on a handle initialized via
  `bno085_init()`
- **THEN** the `RST` pin is driven low for at least `BNO085_RESET_PULSE_MS`
  milliseconds
- **AND** the `RST` pin is then driven high

### Requirement: Wait for data-ready and read SHTP packet
After releasing reset, `bno085_bringup()` SHALL poll the `INT` GPIO pin for
it to go low (active), with a timeout of `BNO085_INT_TIMEOUT_MS`. If `INT`
goes low within the timeout, the driver SHALL assert `CS` low, perform a
single `BNO085_BRINGUP_BUF_SIZE`-byte full-duplex SPI transfer (transmitting
all-zero bytes), then release `CS` high. The driver SHALL parse the first 4
received bytes as the SHTP header: a 16-bit length (LSB first, with the
continuation bit masked off) into `shtp_length`, followed by one byte each
into `shtp_channel` and `shtp_sequence`. The full received buffer SHALL be
stored in the handle.

#### Scenario: Successful read populates the handle and returns HAL_OK
- **WHEN** `bno085_bringup()` is called, `INT` goes low within
  `BNO085_INT_TIMEOUT_MS` of releasing `RST`, and the SPI transfer succeeds
- **THEN** `bno085_bringup()` returns `HAL_OK`
- **AND** the handle's `shtp_length`, `shtp_channel`, and `shtp_sequence`
  fields reflect the first 4 bytes received
- **AND** the handle's raw receive buffer contains all
  `BNO085_BRINGUP_BUF_SIZE` bytes received over SPI

#### Scenario: INT timeout returns an error without an SPI transaction
- **WHEN** `bno085_bringup()` is called and `INT` does not go low within
  `BNO085_INT_TIMEOUT_MS` of releasing `RST`
- **THEN** `bno085_bringup()` returns `HAL_TIMEOUT`
- **AND** no SPI transfer is performed
- **AND** `CS` remains high (deasserted)

#### Scenario: SPI failure during the read is propagated
- **WHEN** `INT` goes low within the timeout, but the
  `HAL_SPI_TransmitReceive()` call fails (error or timeout)
- **THEN** `bno085_bringup()` returns the corresponding non-`HAL_OK`
  `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
