# bno085-get-feature-enum

## Purpose

Sends an SHTP packet from host to device on a given channel and uses the
SH-2 Get Feature Request/Response pair (report IDs `0xFE`/`0xFC`) on the
SHTP control channel to query and report a sensor's current
configuration (report interval, batch interval, change sensitivity,
sensor-specific config, and feature flags), enumerating which sensors
are currently enabled.

## Requirements

### Requirement: Wake the device and wait for INT before a transaction
The driver SHALL provide a helper (`bno085_wake_and_wait_int_low()`)
that is called before any SPI transaction in `bno085_send_packet()` and
`bno085_read_response()`. If `INT` is already low, it returns `HAL_OK`
immediately. Otherwise it pulses `PS0`/`WAKE` low (asking the device to
assert `INT`), waits for `INT` to go low within `BNO085_INT_TIMEOUT_MS`
(via the same poll used by `bno085_reset_and_wait()`), then returns
`PS0`/`WAKE` high before returning the wait result. `bno085_init()`
SHALL accept and store a GPIO port/pin for `PS0`/`WAKE`, and
`bno085_reset_and_wait()` SHALL drive it high at the start of reset (it
must be high from before reset until after the first assertion of
`INT` to select the SPI interface).

#### Scenario: INT already low requires no wake pulse
- **WHEN** `bno085_wake_and_wait_int_low()` is called and `INT` is
  already low
- **THEN** it returns `HAL_OK` without changing `PS0`/`WAKE`

#### Scenario: INT high triggers a wake pulse
- **WHEN** `bno085_wake_and_wait_int_low()` is called and `INT` is high
- **THEN** `PS0`/`WAKE` is driven low, then `INT` is polled for up to
  `BNO085_INT_TIMEOUT_MS`
- **AND** `PS0`/`WAKE` is returned high before the function returns,
  regardless of whether `INT` went low in time
- **AND** the function returns `HAL_OK` if `INT` went low, or
  `HAL_TIMEOUT` otherwise

### Requirement: Send an SHTP packet to a channel
The driver SHALL provide `bno085_send_packet()`, which first calls
`bno085_wake_and_wait_int_low()`; if that does not return `HAL_OK`, it
returns that status without performing any SPI transfer. Otherwise it
builds a 4-byte SHTP header (length = 4 plus the payload length, the
given channel number, and the channel's current host TX sequence number
from `tx_seq[]`), asserts `CS`, performs a single full-duplex
`HAL_SPI_TransmitReceive()` of exactly `4 + payload_len` bytes (the
header followed by the payload), and releases `CS`. SPI is
full-duplex: the device may simultaneously be sending a queued packet
of its own, so the bytes received during this transfer are captured
into `cmd_buf` (rather than discarded) for debugging visibility, and
`cmd_len` is set from the received SHTP header's length field (capped
to `BNO085_CMD_BUF_SIZE`) - `cmd_buf` beyond the `4 + payload_len`
bytes actually transferred is not populated by this call.
`tx_seq[channel]` is incremented (with `uint8_t` wraparound).

#### Scenario: Successful packet send
- **WHEN** `bno085_send_packet()` is called with a channel, payload
  pointer, and payload length, `bno085_wake_and_wait_int_low()` returns
  `HAL_OK`, and the SPI transfer succeeds
- **THEN** `bno085_send_packet()` returns `HAL_OK`
- **AND** the bytes transmitted are a 4-byte SHTP header (length,
  channel, sequence number) followed by the payload bytes
- **AND** `cmd_buf[0..min(4 + payload_len, cmd_len))` contains the
  bytes the device sent back during the transfer, and `cmd_len` is the
  device's reported packet length capped to `BNO085_CMD_BUF_SIZE`
- **AND** `tx_seq[channel]` is incremented by one (wrapping at 256)

#### Scenario: INT timeout during wake is propagated
- **WHEN** `bno085_wake_and_wait_int_low()` returns `HAL_TIMEOUT`
- **THEN** `bno085_send_packet()` returns `HAL_TIMEOUT` without
  asserting `CS` or performing an SPI transfer
- **AND** `tx_seq[channel]` is not incremented

#### Scenario: SPI failure during send is propagated
- **WHEN** the `HAL_SPI_TransmitReceive()` call fails (error or
  timeout)
- **THEN** `bno085_send_packet()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
- **AND** `tx_seq[channel]` is not incremented

### Requirement: Query a sensor's current configuration with Get Feature
The driver SHALL provide `bno085_get_feature()`, which sends an SH-2
Get Feature Request (`{0xFE, report_id, 0x00, 0x00, 0x00, 0x00}`) on
`BNO085_CHANNEL_CONTROL` via `bno085_send_packet()`. Then, up to
`BNO085_GET_FEATURE_MAX_ATTEMPTS` times, it: wakes the device (if
needed) and waits for `INT` to go low within `BNO085_INT_TIMEOUT_MS`
(via `bno085_wake_and_wait_int_low()`); reads the 4-byte SHTP header into
`cmd_buf[0..4)`; computes `cmd_len = min(length, BNO085_CMD_BUF_SIZE)`
from it; and reads exactly `cmd_len - 4` more bytes (if any) into
`cmd_buf[4..cmd_len)` (matching the exact-length read used by
`bno085_read_advertisement()`, so the device's queue is not left
misaligned for subsequent reads). The response to a Get Feature Request
is not always the next packet read - a previously-queued packet (e.g.
the response to an earlier Get Feature Request) may be read first - so
each packet whose payload is not a Get Feature Response (`0xFC`) for
`report_id` is discarded and another packet is read. Once a packet has
`cmd_len >= 21` and a payload beginning with `0xFC` followed by
`report_id`, the remaining 15 bytes SHALL be parsed little-endian into
`feature` (a `bno085_feature_t`): `feature_flags` (1 byte),
`change_sensitivity` (2 bytes), `report_interval_us` (4 bytes),
`batch_interval_us` (4 bytes), `sensor_specific_config` (4 bytes), with
`feature_report_id` set to `report_id`.

#### Scenario: Successful Get Feature query
- **WHEN** `bno085_get_feature()` is called with a `report_id`, and
  within `BNO085_GET_FEATURE_MAX_ATTEMPTS` reads (each preceded by
  `INT` going low within `BNO085_INT_TIMEOUT_MS`), one of the packets
  read is at least 21 bytes with a payload of `0xFC` followed by
  `report_id`
- **THEN** `bno085_get_feature()` returns `HAL_OK`
- **AND** `feature.feature_report_id` equals `report_id`
- **AND** `feature.report_interval_us`, `feature.batch_interval_us`,
  `feature.change_sensitivity`, `feature.sensor_specific_config`, and
  `feature.feature_flags` are populated from `cmd_buf` as
  little-endian fields

#### Scenario: INT timeout returns an error without reading a response
- **WHEN** `bno085_get_feature()` is called and `INT` does not go low
  within `BNO085_INT_TIMEOUT_MS` before a read
- **THEN** `bno085_get_feature()` returns `HAL_TIMEOUT`
- **AND** the corresponding response SPI transfer is not performed

#### Scenario: No matching response within the retry limit is an error
- **WHEN** `BNO085_GET_FEATURE_MAX_ATTEMPTS` packets are read (each with
  `INT` going low and successful SPI transfers), and none of them has
  `cmd_len >= 21` with a payload of `0xFC` followed by `report_id`
- **THEN** `bno085_get_feature()` returns `HAL_ERROR`
- **AND** `cmd_buf`/`cmd_len` retain the last-read packet for
  inspection
- **AND** `feature` is left unmodified

#### Scenario: SPI failure during a read is propagated
- **WHEN** `INT` goes low within the timeout, but either the
  header-read or the payload-read `HAL_SPI_TransmitReceive()` call
  fails (error or timeout)
- **THEN** `bno085_get_feature()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
