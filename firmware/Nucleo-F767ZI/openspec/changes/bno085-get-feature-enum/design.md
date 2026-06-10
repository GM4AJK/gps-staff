## Context

So far the driver only performs SPI *reads* triggered by `INT` going
low (`bno085_bringup()`, `bno085_read_advertisement()`). The SH-2
protocol's "Get Feature Request" (report ID `0xFE`) is sent *to* the
device on the SHTP control channel; the device replies with "Get
Feature Response" (`0xFC`) on the same channel once `INT` goes low.

From the bench-confirmed advertisement:
- channel 2 is `sensorhub:control` - the SH-2 command/control channel
- channel 3 is `sensorhub:inputNormal` - where unsolicited sensor
  reports normally arrive
- the advertisement's `tag 129` table lists `(report_id, max_length)`
  pairs for every report the firmware supports, including `01 0A`
  (Accelerometer), `02 0A` (Gyroscope), `03 0A` (Magnetic Field), `05
  0E` (Rotation Vector), `08 0C` (Game Rotation Vector), confirming
  these report IDs are valid to query

Each SHTP channel has an independent sequence number per direction
(host->device and device->host), starting at 0 and incrementing (with
wraparound) for each packet sent in that direction on that channel.
Until now the driver has only ever read on channel 0 (using whatever
sequence number the device sent), so it has never needed to track its
own outgoing sequence numbers.

## Goals / Non-Goals

**Goals:**
- Add a generic SPI write path (`bno085_send_packet()`) for sending an
  SHTP packet to a given channel, with the driver tracking its own
  per-channel TX sequence numbers.
- Implement `bno085_get_feature()`: send a Get Feature Request for a
  given SH-2 report ID on the control channel, wait for `INT`, read
  and parse the Get Feature Response.
- Demo: query a fixed list of well-known sensor report IDs
  (Accelerometer `0x01`, Gyroscope `0x02`, Magnetic Field `0x03`,
  Rotation Vector `0x05`, Game Rotation Vector `0x08`) and print each
  response over USART3.

**Non-Goals:**
- No Get Feature *Set* (enabling/configuring a sensor's report
  interval) - this change only queries current configuration.
- No dynamic enumeration from the advertisement's `tag 129` table -
  the demo uses a fixed, hand-picked list of report IDs known from the
  bench-captured advertisement. Driving the query list from a parsed
  `tag 129` table is a possible follow-up.
- No handling of unsolicited reports that may arrive on
  `inputNormal`/other channels interleaved with control-channel
  responses (see Risks).

## Decisions

- **`bno085_send_packet()` signature**: takes the handle, a channel
  number, a pointer to payload bytes, and a payload length. It builds
  the 4-byte SHTP header (length = 4 + payload length, channel, current
  TX sequence number for that channel), asserts `CS`, performs one
  `HAL_SPI_TransmitReceive()` of `header + payload` (discarding the
  received bytes), releases `CS`, and increments the channel's TX
  sequence number. No `INT` wait - per SHTP, the host may write
  whenever it chooses.
- **Per-channel TX sequence numbers**: add `uint8_t tx_seq[6]` to
  `bno085_t` (one counter per SHTP channel 0-5, matching the channels
  seen in the advertisement), initialized to 0 by `bno085_init()`
  (already zeroed via `memset`).
- **Command buffer**: add `uint8_t cmd_buf[BNO085_CMD_BUF_SIZE]` with
  `BNO085_CMD_BUF_SIZE = 32` (Get Feature Response is 4-byte header +
  17-byte payload = 21 bytes, so 32 gives margin), separate from
  `rx_buf`/`advert_buf` to keep each function's buffer purpose
  explicit, matching the pattern established for `advert_buf`.
- **`bno085_get_feature()` flow**:
  1. Build a 6-byte Get Feature Request payload: `{0xFE, report_id, 0,
     0, 0, 0}`.
  2. `bno085_send_packet()` on `BNO085_CHANNEL_CONTROL` (= 2).
  3. Wait for `INT` low with `BNO085_INT_TIMEOUT_MS` timeout (same
     pattern as `bno085_reset_and_wait()`'s poll loop, but without the
     `RST` pulse or deassert-debounce - those are reset-specific).
  4. On `INT` low, assert `CS`, `HAL_SPI_TransmitReceive()`
     `BNO085_CMD_BUF_SIZE` bytes into `cmd_buf`, release `CS`.
  5. Parse the SHTP header (bytes 0-3) and, if `cmd_buf[4] == 0xFC`
     (Get Feature Response) and `cmd_buf[5] == report_id`, parse the
     remaining 15 bytes into `bno085_feature_t` and return `HAL_OK`.
     Otherwise return a new sentinel status (see below).
- **`bno085_feature_t` struct fields** (from the Get Feature Response
  payload, all multi-byte fields little-endian): `feature_report_id`
  (`uint8_t`), `feature_flags` (`uint8_t`), `change_sensitivity`
  (`uint16_t`), `report_interval_us` (`uint32_t`), `batch_interval_us`
  (`uint32_t`), `sensor_specific_config` (`uint32_t`). A sensor is
  considered enabled if `report_interval_us != 0`.
- **Mismatch/unexpected response handling**: if the read packet's
  channel isn't `BNO085_CHANNEL_CONTROL`, or `cmd_buf[4]` isn't `0xFC`,
  or `cmd_buf[5]` doesn't match the requested `report_id`,
  `bno085_get_feature()` returns `HAL_ERROR` (the existing
  `HAL_StatusTypeDef` vocabulary already used for SPI failures, with
  `HAL_TIMEOUT` reserved for the `INT` wait). The raw `cmd_buf` remains
  available on the handle for inspection/debugging via USART3 in the
  demo.

## Risks / Trade-offs

- [An unsolicited report (e.g. on `inputNormal`) could arrive and
  assert `INT` before/instead of the Get Feature Response] →
  `bno085_get_feature()` only performs a single read attempt per call
  and returns `HAL_ERROR` on a mismatched response; the demo prints the
  raw `cmd_buf` on mismatch so this is visible on the bench. A
  retry/loop could be added in a follow-up if this proves common in
  practice.
- [Sequence number tracking is host-side only and unvalidated against
  the device's expectations] → if the device rejects packets with
  unexpected sequence numbers, the first Get Feature Request (sequence
  0 on channel 2) should still work since both sides start at 0 after
  reset; this is the primary scenario exercised by the demo.
- [`BNO085_CMD_BUF_SIZE = 32` assumes Get Feature Response (21 bytes)
  is the largest control-channel response queried] → sufficient for
  this change's fixed report-ID list; a larger response would be
  truncated, visible as a short/garbled `cmd_buf` dump.
