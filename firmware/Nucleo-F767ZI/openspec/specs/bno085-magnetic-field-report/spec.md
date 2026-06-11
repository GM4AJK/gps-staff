# bno085-magnetic-field-report

## Purpose

Read and report the BNO085's Magnetic Field Calibrated (0x03) input
report alongside the existing Rotation Vector report, for bench
verification of the sensor's calibrated magnetometer output.

## Requirements

### Requirement: Enable the Magnetic Field Calibrated report
The driver SHALL enable the SH-2 Magnetic Field Calibrated (0x03) input
report via `bno085_set_feature()`, at the same report interval used for
the Rotation Vector (0x05) report.

#### Scenario: Magnetic Field Calibrated report enabled at startup
- **WHEN** `app_init()` runs and `bno085_set_feature()` is called for
  report ID `0x03` with a 50000us (20Hz) interval
- **THEN** `bno085_set_feature()` returns `HAL_OK` and the result is
  printed over USART3

### Requirement: Read a Magnetic Field Calibrated input report
The driver SHALL provide `bno085_read_magnetic_field()`, which loops
`bno085_read_response()` up to `BNO085_GET_FEATURE_MAX_ATTEMPTS` times
looking for a packet on `BNO085_CHANNEL_INPUT_REPORTS` with `cmd_len >=
19`, `cmd_buf[4] == BNO085_REPORT_ID_BASE_TIMESTAMP`, and `cmd_buf[9] ==
0x03` (Magnetic Field Calibrated). On a match, it SHALL parse
`cmd_buf[10..19)` into `p->magnetic_field`: `sequence` from
`cmd_buf[10]`, `status` from the low 2 bits of `cmd_buf[11]`, and
`x`/`y`/`z` as little-endian signed 16-bit values from `cmd_buf[13..14]`,
`cmd_buf[15..16]`, `cmd_buf[17..18]` respectively (Q point 4). It SHALL
also compute `x_f`/`y_f`/`z_f` as the corresponding float values in
uTesla.

#### Scenario: Successful Magnetic Field Calibrated read
- **WHEN** `bno085_read_magnetic_field()` is called and, within
  `BNO085_GET_FEATURE_MAX_ATTEMPTS` reads, a packet matching the above
  conditions is read
- **THEN** `bno085_read_magnetic_field()` returns `HAL_OK`
- **AND** `p->magnetic_field.sequence`, `.status`, `.x`, `.y`, `.z`,
  `.x_f`, `.y_f`, `.z_f` are populated from that packet

#### Scenario: No matching packet returns an error
- **WHEN** `bno085_read_response()` returns a non-`HAL_OK` status, or
  `BNO085_GET_FEATURE_MAX_ATTEMPTS` packets are read and none matches
- **THEN** `bno085_read_magnetic_field()` returns that status (or
  `HAL_ERROR` if none matched)
- **AND** `p->magnetic_field` is left unmodified

### Requirement: Print a Magnetic Field Calibrated report
The driver SHALL provide `bno085_print_magnetic_field()`, which prints
`p->magnetic_field`'s `x_f`/`y_f`/`z_f` (in uT, formatted to 4 decimal
places via the existing fixed-point `bno085_format_q()`-style helper),
`sequence`, and `status` over the given UART, in the form:
`mag: x=<x_f> y=<y_f> z=<z_f> seq=<sequence> status=<status>\r\n`.

#### Scenario: Print after a successful read
- **WHEN** `bno085_print_magnetic_field()` is called after
  `bno085_read_magnetic_field()` returned `HAL_OK`
- **THEN** a line of the form `mag: x=<value> y=<value> z=<value>
  seq=<n> status=<0-3>\r\n` is transmitted over the given UART

### Requirement: Wire Magnetic Field reading into the application loop
`app_loop()` SHALL call `bno085_read_magnetic_field()` and, on
`HAL_OK`, `bno085_print_magnetic_field()`, alongside the existing
Rotation Vector read/print.

#### Scenario: Magnetic field printed alongside rotation vector
- **WHEN** `app_loop()` runs an iteration and both
  `bno085_read_rotation_vector()` and `bno085_read_magnetic_field()`
  return `HAL_OK`
- **THEN** both an `rv: ...` line and a `mag: ...` line are transmitted
  over USART3 during that iteration
