## ADDED Requirements

### Requirement: Send an SHTP packet to a channel
The driver SHALL provide `bno085_send_packet()`, which builds a 4-byte
SHTP header (length = 4 plus the payload length, the given channel
number, and the channel's current host TX sequence number from
`tx_seq[]`), asserts `CS`, and performs a full-duplex transfer of the
header followed by the payload. SPI is full-duplex: the device may
simultaneously be sending a queued packet of its own, so the bytes
received during this transfer are captured into `cmd_buf` rather than
discarded. The first 4 received bytes are the device's own pending
SHTP header; their length field gives `rx_total = min(length,
BNO085_CMD_BUF_SIZE)`. The transfer is extended to exactly
`max(4 + payload_len, rx_total)` bytes total (extra bytes on the host's
side are zero-padded), so neither the host's request nor the device's
queued packet is truncated. `cmd_len` is set to `rx_total`. `CS` is
released, and `tx_seq[channel]` is incremented (with `uint8_t`
wraparound).

#### Scenario: Successful packet send
- **WHEN** `bno085_send_packet()` is called with a channel, payload
  pointer, and payload length, and the SPI transfer(s) succeed
- **THEN** `bno085_send_packet()` returns `HAL_OK`
- **AND** the bytes transmitted begin with a 4-byte SHTP header
  (length, channel, sequence number) followed by the payload bytes
  (and zero padding if the device's queued packet is longer)
- **AND** `cmd_buf[0..cmd_len)` contains the SHTP packet the device
  sent back during the transfer, and `cmd_len` is the device's reported
  length capped to `BNO085_CMD_BUF_SIZE`
- **AND** `tx_seq[channel]` is incremented by one (wrapping at 256)

#### Scenario: SPI failure during send is propagated
- **WHEN** either the header or payload `HAL_SPI_TransmitReceive()`
  call fails (error or timeout)
- **THEN** `bno085_send_packet()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
- **AND** `tx_seq[channel]` is not incremented

### Requirement: Query a sensor's current configuration with Get Feature
The driver SHALL provide `bno085_get_feature()`, which sends an SH-2
Get Feature Request (`{0xFE, report_id, 0x00, 0x00, 0x00, 0x00}`) on
`BNO085_CHANNEL_CONTROL` via `bno085_send_packet()`. The packet
captured by `bno085_send_packet()` into `cmd_buf`/`cmd_len` is checked
first. If it is not a match, up to `BNO085_GET_FEATURE_MAX_ATTEMPTS - 1`
further packets are read: each read waits for `INT` to go low within
`BNO085_INT_TIMEOUT_MS`, then reads the 4-byte SHTP header into
`cmd_buf[0..4)`, computes `cmd_len = min(length, BNO085_CMD_BUF_SIZE)`
from it, and reads exactly `cmd_len - 4` more bytes (if any) into
`cmd_buf[4..cmd_len)` (matching the exact-length read used by
`bno085_read_advertisement()`, so the device's queue is not left
misaligned for subsequent reads). The response to a Get Feature Request
is not always the first packet checked - a previously-queued packet
(e.g. the response to an earlier Get Feature Request) may be read
first - so each packet whose payload is not a Get Feature Response
(`0xFC`) for `report_id` is discarded and another packet is read. Once
a packet has `cmd_len >= 21` and a payload beginning with `0xFC`
followed by `report_id`, the remaining 15 bytes SHALL be parsed
little-endian into `feature` (a `bno085_feature_t`): `feature_flags`
(1 byte), `change_sensitivity` (2 bytes), `report_interval_us` (4
bytes), `batch_interval_us` (4 bytes), `sensor_specific_config` (4
bytes), with `feature_report_id` set to `report_id`.

#### Scenario: Successful Get Feature query
- **WHEN** `bno085_get_feature()` is called with a `report_id`, and
  the packet captured by `bno085_send_packet()` or one of up to
  `BNO085_GET_FEATURE_MAX_ATTEMPTS - 1` subsequent reads (each preceded
  by `INT` going low within `BNO085_INT_TIMEOUT_MS`) is at least 21
  bytes with a payload of `0xFC` followed by `report_id`
- **THEN** `bno085_get_feature()` returns `HAL_OK`
- **AND** `feature.feature_report_id` equals `report_id`
- **AND** `feature.report_interval_us`, `feature.batch_interval_us`,
  `feature.change_sensitivity`, `feature.sensor_specific_config`, and
  `feature.feature_flags` are populated from `cmd_buf` as
  little-endian fields

#### Scenario: INT timeout on a retry read returns an error
- **WHEN** the packet captured by `bno085_send_packet()` is not a
  match, and `INT` does not go low within `BNO085_INT_TIMEOUT_MS`
  before a retry read
- **THEN** `bno085_get_feature()` returns `HAL_TIMEOUT`
- **AND** the corresponding response SPI transfer is not performed

#### Scenario: No matching response within the retry limit is an error
- **WHEN** the packet captured by `bno085_send_packet()` plus
  `BNO085_GET_FEATURE_MAX_ATTEMPTS - 1` subsequent reads (each with
  `INT` going low and successful SPI transfers) are checked, and none
  of them has `cmd_len >= 21` with a payload of `0xFC` followed by
  `report_id`
- **THEN** `bno085_get_feature()` returns `HAL_ERROR`
- **AND** `cmd_buf`/`cmd_len` retain the last-read packet for
  inspection
- **AND** `feature` is left unmodified

#### Scenario: SPI failure during a retry read is propagated
- **WHEN** `INT` goes low within the timeout, but either the
  header-read or the payload-read `HAL_SPI_TransmitReceive()` call
  fails (error or timeout)
- **THEN** `bno085_get_feature()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning
