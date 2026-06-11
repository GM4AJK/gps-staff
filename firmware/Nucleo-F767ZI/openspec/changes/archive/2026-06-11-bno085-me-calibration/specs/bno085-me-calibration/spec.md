## ADDED Requirements

### Requirement: Send an SH-2 Command Request
The driver SHALL provide `bno085_send_command()`, taking `command` (1-127)
and a 9-byte parameter array (`P0..P8`). It SHALL build a 12-byte SH-2
Command Request payload (`{0xF2, p->cmd_seq, command, P0, ..., P8}`) and
send it on `BNO085_CHANNEL_CONTROL` via `bno085_send_packet()`. On success,
it SHALL record the sequence number used in `p->last_cmd_seq` and increment
`p->cmd_seq` (with `uint8_t` wraparound).

#### Scenario: Successful command send
- **WHEN** `bno085_send_command()` is called with `command` and a parameter
  array, and `bno085_send_packet()` succeeds
- **THEN** `bno085_send_command()` returns `HAL_OK`
- **AND** the 12-byte payload sent is `0xF2`, the sequence number that was
  in `p->cmd_seq` before the call, `command`, then `P0..P8` in order
- **AND** `p->last_cmd_seq` equals the sequence number that was sent
- **AND** `p->cmd_seq` is incremented by one (wrapping at 256)

#### Scenario: Send failure is propagated without incrementing the sequence
- **WHEN** `bno085_send_packet()` returns a non-`HAL_OK` status
- **THEN** `bno085_send_command()` returns that status
- **AND** neither `p->cmd_seq` nor `p->last_cmd_seq` is modified

### Requirement: Read an SH-2 Command Response
The driver SHALL provide `bno085_read_command_response()`, taking
`command`. Up to `BNO085_GET_FEATURE_MAX_ATTEMPTS` times, it SHALL: wake the
device (if needed) and wait for `INT` to go low within
`BNO085_INT_TIMEOUT_MS`, then read a packet via `bno085_read_response()`
into `cmd_buf`/`cmd_len`. A packet matches if `cmd_len >= 20`,
`cmd_buf[4] == 0xF1`, `cmd_buf[6] == command`, and `cmd_buf[7] ==
p->last_cmd_seq`. Each non-matching packet SHALL be discarded and another
read. On a match, `cmd_buf[9..20)` (`R0..R10`) are available for the caller
to interpret; `bno085_read_command_response()` itself does not parse them
further.

#### Scenario: Successful command response read
- **WHEN** `bno085_read_command_response()` is called for `command` and,
  within `BNO085_GET_FEATURE_MAX_ATTEMPTS` reads (each preceded by `INT`
  going low within `BNO085_INT_TIMEOUT_MS`), one of the packets read is at
  least 20 bytes with payload `0xF1`, `<seq>`, `command`,
  `p->last_cmd_seq`, `<response_seq>`, `R0..R10`
- **THEN** `bno085_read_command_response()` returns `HAL_OK`
- **AND** `cmd_buf[9..20)` contains `R0..R10` from that packet

#### Scenario: Non-matching packets are discarded
- **WHEN** a packet read by `bno085_read_response()` does not satisfy the
  length/report-ID/command/sequence match conditions (e.g. an unsolicited
  response, a response to a different command, or a sensor input report)
- **THEN** it is discarded and `bno085_read_command_response()` reads
  another packet, up to `BNO085_GET_FEATURE_MAX_ATTEMPTS` total reads

#### Scenario: INT timeout or no matching packet returns an error
- **WHEN** `INT` does not go low within `BNO085_INT_TIMEOUT_MS` before a
  read
- **THEN** `bno085_read_command_response()` returns `HAL_TIMEOUT`
- **WHEN** instead `BNO085_GET_FEATURE_MAX_ATTEMPTS` packets are read (each
  with `INT` going low and successful SPI transfers) and none matches
- **THEN** `bno085_read_command_response()` returns `HAL_ERROR`, and
  `cmd_buf`/`cmd_len` retain the last-read packet for inspection

### Requirement: Query ME calibration enable state
The driver SHALL provide `bno085_get_me_calibration()`, which sends a Get ME
Calibration command (`bno085_send_command()` with `command = 0x07` and
parameters `{0, 0, 0, 0x01, 0, 0, 0, 0, 0}`), then calls
`bno085_read_command_response(p, 0x07)`. On `HAL_OK`, it SHALL parse
`cmd_buf[9..15)` into `me_calibration`: `status` from `R0`, and
`accel_enable`, `gyro_enable`, `mag_enable`, `planar_accel_enable`,
`on_table_enable` from `R1..R5` respectively.

#### Scenario: Successful Get ME Calibration query
- **WHEN** `bno085_get_me_calibration()` is called and
  `bno085_read_command_response(p, 0x07)` returns `HAL_OK` for the request
  sent with `P3 = 0x01`
- **THEN** `bno085_get_me_calibration()` returns `HAL_OK`
- **AND** `me_calibration.status`, `.accel_enable`, `.gyro_enable`,
  `.mag_enable`, `.planar_accel_enable`, and `.on_table_enable` are
  populated from `cmd_buf[9..15)`

#### Scenario: Send or response failure is propagated
- **WHEN** `bno085_send_command()` or `bno085_read_command_response()`
  returns a non-`HAL_OK` status
- **THEN** `bno085_get_me_calibration()` returns that status
- **AND** `me_calibration` is left unmodified

### Requirement: Configure ME calibration enable state
The driver SHALL provide `bno085_set_me_calibration()`, taking
`accel_enable`, `gyro_enable`, `mag_enable`, `planar_accel_enable`, and
`on_table_enable` (each `uint8_t`, 0 or 1). It SHALL send a Configure ME
Calibration command (`bno085_send_command()` with `command = 0x07` and
parameters `{accel_enable, gyro_enable, mag_enable, 0x00,
planar_accel_enable, on_table_enable, 0, 0, 0}`), then call
`bno085_read_command_response(p, 0x07)`. On `HAL_OK`, it SHALL parse
`cmd_buf[9..15)` into `me_calibration` exactly as
`bno085_get_me_calibration()` does.

#### Scenario: Successful Configure ME Calibration
- **WHEN** `bno085_set_me_calibration()` is called with enable flags and
  `bno085_read_command_response(p, 0x07)` returns `HAL_OK` for the request
  sent with `P3 = 0x00`
- **THEN** `bno085_set_me_calibration()` returns `HAL_OK`
- **AND** the 12-byte Command Request payload's `P0..P2` and `P4..P5` equal
  `accel_enable`, `gyro_enable`, `mag_enable`, `planar_accel_enable`, and
  `on_table_enable` respectively, and `P3 == 0x00`
- **AND** `me_calibration.status`, `.accel_enable`, `.gyro_enable`,
  `.mag_enable`, `.planar_accel_enable`, and `.on_table_enable` are
  populated from `cmd_buf[9..15)`

#### Scenario: Send or response failure is propagated
- **WHEN** `bno085_send_command()` or `bno085_read_command_response()`
  returns a non-`HAL_OK` status
- **THEN** `bno085_set_me_calibration()` returns that status
- **AND** `me_calibration` is left unmodified

### Requirement: Save dynamic calibration data (DCD)
The driver SHALL provide `bno085_save_dcd()`, which sends a Save DCD command
(`bno085_send_command()` with `command = 0x06` and all-zero parameters),
then calls `bno085_read_command_response(p, 0x06)`. On `HAL_OK`, it SHALL
set `me_calibration.status` from `cmd_buf[9]` (`R0`).

#### Scenario: Successful Save DCD
- **WHEN** `bno085_save_dcd()` is called and
  `bno085_read_command_response(p, 0x06)` returns `HAL_OK`
- **THEN** `bno085_save_dcd()` returns `HAL_OK`
- **AND** `me_calibration.status` equals `cmd_buf[9]` (0 = success)

#### Scenario: Send or response failure is propagated
- **WHEN** `bno085_send_command()` or `bno085_read_command_response()`
  returns a non-`HAL_OK` status
- **THEN** `bno085_save_dcd()` returns that status
