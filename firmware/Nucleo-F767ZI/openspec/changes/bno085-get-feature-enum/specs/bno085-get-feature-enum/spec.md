## ADDED Requirements

### Requirement: Send an SHTP packet to a channel
The driver SHALL provide `bno085_send_packet()`, which builds a 4-byte
SHTP header (length = 4 plus the payload length, the given channel
number, and the channel's current host TX sequence number from
`tx_seq[]`), waits for `INT` to go low within `BNO085_INT_TIMEOUT_MS`
(per the BNO08x SPI protocol, the host SHALL NOT initiate an SPI
transaction - read or write - until the device asserts `INT`), then
asserts `CS`, performs a single `HAL_SPI_TransmitReceive()` of the
header followed by the payload bytes (received bytes discarded),
releases `CS`, and increments `tx_seq[channel]` (with `uint8_t`
wraparound).

#### Scenario: Successful packet send
- **WHEN** `bno085_send_packet()` is called with a channel, payload
  pointer, and payload length, `INT` goes low within
  `BNO085_INT_TIMEOUT_MS`, and the SPI transfer succeeds
- **THEN** `bno085_send_packet()` returns `HAL_OK`
- **AND** the bytes transmitted are a 4-byte SHTP header (length,
  channel, sequence number) followed by the payload bytes
- **AND** `tx_seq[channel]` is incremented by one (wrapping at 256)

#### Scenario: INT timeout returns an error without an SPI transaction
- **WHEN** `bno085_send_packet()` is called and `INT` does not go low
  within `BNO085_INT_TIMEOUT_MS`
- **THEN** `bno085_send_packet()` returns `HAL_TIMEOUT`
- **AND** no SPI transfer is performed
- **AND** `tx_seq[channel]` is not incremented

#### Scenario: SPI failure during send is propagated
- **WHEN** `INT` goes low within the timeout, but the
  `HAL_SPI_TransmitReceive()` call fails (error or timeout)
- **THEN** `bno085_send_packet()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
- **AND** `tx_seq[channel]` is not incremented

### Requirement: Query a sensor's current configuration with Get Feature
The driver SHALL provide `bno085_get_feature()`, which sends an SH-2
Get Feature Request (`{0xFE, report_id, 0x00, 0x00, 0x00, 0x00}`) on
`BNO085_CHANNEL_CONTROL` via `bno085_send_packet()`, then waits again
for `INT` to go low within `BNO085_INT_TIMEOUT_MS` for the response.
On `INT` low, it asserts `CS`, reads the 4-byte SHTP header into
`cmd_buf[0..4)`, computes `total_len = min(length, BNO085_CMD_BUF_SIZE)`
from it, then reads exactly `total_len - 4` more bytes (if any) into
`cmd_buf[4..total_len)`, before releasing `CS` (matching the
exact-length read used by `bno085_read_advertisement()`, so the
device's queue is not left misaligned for subsequent reads). If
`total_len >= 21` and the response payload begins with `0xFC` (Get
Feature Response) and the following byte equals `report_id`, the
remaining 15 bytes SHALL be parsed little-endian into `feature` (a
`bno085_feature_t`): `feature_flags` (1 byte), `change_sensitivity` (2
bytes), `report_interval_us` (4 bytes), `batch_interval_us` (4 bytes),
`sensor_specific_config` (4 bytes), with `feature_report_id` set to
`report_id`.

#### Scenario: Successful Get Feature query
- **WHEN** `bno085_get_feature()` is called with a `report_id`, `INT`
  goes low within `BNO085_INT_TIMEOUT_MS` after the request is sent,
  and the response is a 21-byte packet whose payload is `0xFC`
  followed by `report_id`
- **THEN** `bno085_get_feature()` returns `HAL_OK`
- **AND** `feature.feature_report_id` equals `report_id`
- **AND** `feature.report_interval_us`, `feature.batch_interval_us`,
  `feature.change_sensitivity`, `feature.sensor_specific_config`, and
  `feature.feature_flags` are populated from `cmd_buf` as
  little-endian fields

#### Scenario: INT timeout returns an error without reading a response
- **WHEN** `bno085_get_feature()` is called and `INT` does not go low
  within `BNO085_INT_TIMEOUT_MS` after the request is sent
- **THEN** `bno085_get_feature()` returns `HAL_TIMEOUT`
- **AND** no response SPI transfer is performed

#### Scenario: Unexpected response is reported as an error
- **WHEN** `INT` goes low and the response SPI transfers succeed, but
  `total_len < 21`, or the response payload does not begin with `0xFC`,
  or the following byte does not equal the requested `report_id`
- **THEN** `bno085_get_feature()` returns `HAL_ERROR`
- **AND** `cmd_buf` retains the raw received bytes for inspection
- **AND** `feature` is left unmodified

#### Scenario: SPI failure during the response read is propagated
- **WHEN** `INT` goes low within the timeout, but either the
  header-read or the payload-read `HAL_SPI_TransmitReceive()` call
  fails (error or timeout)
- **THEN** `bno085_get_feature()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
