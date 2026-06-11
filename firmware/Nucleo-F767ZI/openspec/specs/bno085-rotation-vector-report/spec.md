# bno085-rotation-vector-report

## Purpose

Read and parse the SH-2 Rotation Vector input report (report ID `0x05`)
from the BNO085's input-report channel, and print the resulting
quaternion and accuracy values over UART for diagnostic use.

## Requirements

### Requirement: Read and parse a Rotation Vector input report
The driver SHALL provide `bno085_read_rotation_vector()`. Up to
`BNO085_GET_FEATURE_MAX_ATTEMPTS` times, it SHALL: wake the device (if
needed) and wait for `INT` to go low within `BNO085_INT_TIMEOUT_MS`, then
read a packet via `bno085_read_response()` (the same exact-length two-step
read used by `bno085_get_feature()`, populating `cmd_buf`/`cmd_len`). A
packet matches if `cmd_buf[2] == BNO085_CHANNEL_INPUT_REPORTS`,
`cmd_len >= 23`, `cmd_buf[4] == 0xFB` (Base Timestamp Reference), and
`cmd_buf[9] == 0x05` (Rotation Vector report ID). Each non-matching packet
SHALL be discarded and another packet read.

When a matching packet is found, the 14-byte Rotation Vector report at
`cmd_buf[9..23)` SHALL be parsed into `rotation_vector` (a
`bno085_rotation_vector_t`):
- `sequence` from byte 1 (`cmd_buf[10]`)
- `status` from byte 2 (`cmd_buf[11]`), masked to bits 1:0
- `i`, `j`, `k`, `real` as signed 16-bit little-endian Q14 values from bytes
  4-11 (`cmd_buf[13..21)`), with corresponding `i_f`, `j_f`, `k_f`, `real_f`
  float fields computed as `value / 16384.0f`
- `accuracy` as a signed 16-bit little-endian Q12 value from bytes 12-13
  (`cmd_buf[21..23)`), with a corresponding `accuracy_rad` float field
  computed as `value / 4096.0f`

#### Scenario: Successful Rotation Vector read
- **WHEN** `bno085_read_rotation_vector()` is called and, within
  `BNO085_GET_FEATURE_MAX_ATTEMPTS` reads (each preceded by `INT` going low
  within `BNO085_INT_TIMEOUT_MS`), one of the packets read is on
  `BNO085_CHANNEL_INPUT_REPORTS`, at least 23 bytes, with payload
  `0xFB ... 0x05 ...` (Base Timestamp Reference followed by Rotation Vector)
- **THEN** `bno085_read_rotation_vector()` returns `HAL_OK`
- **AND** `rotation_vector.i`, `.j`, `.k`, `.real`, and `.accuracy` are
  populated as signed Q14/Q12 integers from `cmd_buf`
- **AND** `rotation_vector.i_f`, `.j_f`, `.k_f`, `.real_f`, and
  `.accuracy_rad` are the corresponding float conversions
- **AND** `rotation_vector.sequence` and `.status` are populated from
  `cmd_buf`

#### Scenario: Non-matching packets are discarded
- **WHEN** a packet read by `bno085_read_response()` does not satisfy the
  channel/length/report-ID match conditions (e.g. it is a control-channel
  response or a different sensor's report)
- **THEN** it is discarded and `bno085_read_rotation_vector()` reads
  another packet, up to `BNO085_GET_FEATURE_MAX_ATTEMPTS` total reads

#### Scenario: INT timeout returns an error without reading a response
- **WHEN** `bno085_read_rotation_vector()` is called and `INT` does not go
  low within `BNO085_INT_TIMEOUT_MS` before a read
- **THEN** `bno085_read_rotation_vector()` returns `HAL_TIMEOUT`
- **AND** the corresponding response SPI transfer is not performed

#### Scenario: No matching packet within the retry limit is an error
- **WHEN** `BNO085_GET_FEATURE_MAX_ATTEMPTS` packets are read (each with
  `INT` going low and successful SPI transfers), and none of them matches
- **THEN** `bno085_read_rotation_vector()` returns `HAL_ERROR`
- **AND** `cmd_buf`/`cmd_len` retain the last-read packet for inspection
- **AND** `rotation_vector` is left unmodified

#### Scenario: SPI failure during a read is propagated
- **WHEN** `INT` goes low within the timeout, but either the header-read or
  the payload-read `HAL_SPI_TransmitReceive()` call fails (error or
  timeout)
- **THEN** `bno085_read_rotation_vector()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning

### Requirement: Print a parsed Rotation Vector
The driver SHALL provide `bno085_print_rotation_vector()`, which transmits
a single line over the given UART summarizing `rotation_vector`: the
quaternion components and accuracy as floats (`i_f`, `j_f`, `k_f`, `real_f`,
`accuracy_rad`), and the `sequence` and `status` fields, following the
`HAL_UART_Transmit()` pattern used by `bno085_print_advertisement()`.

#### Scenario: Print after a successful read
- **WHEN** `bno085_print_rotation_vector()` is called after a successful
  `bno085_read_rotation_vector()`
- **THEN** one line is transmitted over the given UART containing the
  quaternion's `i_f`, `j_f`, `k_f`, `real_f`, `accuracy_rad`, `sequence`,
  and `status` values
