## Context

`bno085_get_feature()` already implements the request/response dance for the
SH-2 control channel: send a packet via `bno085_send_packet()`, then loop
`bno085_read_response()` (which performs the exact-length two-step SPI read
used throughout this driver) up to `BNO085_GET_FEATURE_MAX_ATTEMPTS` times,
discarding non-matching packets, until a matching response is found. Both the
Set Feature Command and the Rotation Vector input report can reuse this same
plumbing - `bno085_read_response()` already captures whichever channel's
packet comes back next into `cmd_buf`/`cmd_len`, regardless of channel.

Per the SH-2 Reference Manual (1000-3625 v1.9, now in
`docs/datasheets/SH-2-Reference-Manual.pdf`):
- Set Feature Command (0xFD), section 6.5.4: 17-byte control-channel payload
  - byte 1: feature report ID
  - byte 2: feature flags
  - bytes 3-4: change sensitivity (u16 LE)
  - bytes 5-8: report interval (u32 LE, microseconds)
  - bytes 9-12: batch interval (u32 LE, microseconds)
  - bytes 13-16: sensor-specific configuration (u32 LE)
- Sensor input reports arrive on SHTP channel 3 ("Wakeup/Normal" input
  reports). A batch typically begins with a Base Timestamp Reference (0xFB,
  5 bytes), followed by one or more sensor reports.
- Rotation Vector (0x05) input report, section 6.5.18: 14 bytes - report ID,
  sequence number, status, delay, then i/j/k/real quaternion components
  (s16 LE, Q point 14) and a heading accuracy estimate (s16 LE, Q point 12).

## Goals / Non-Goals

**Goals:**
- Enable a sensor report (specifically Rotation Vector) via Set Feature.
- Read and decode the resulting Rotation Vector input reports into the
  `bno085_t` struct as both raw Q-point integers and floats.

**Non-Goals:**
- General-purpose dispatch for arbitrary sensor report types - this change
  only handles the Rotation Vector (0x05) report shape.
- Handling Timestamp Rebase (0xFA) records or multi-sensor batches in the
  same packet - if encountered, they are treated as non-matching and
  discarded like any other unrecognized packet.

## Decisions

- **`bno085_set_feature()` mirrors `bno085_get_feature()`**: builds the
  17-byte Set Feature Command payload from caller-supplied arguments
  (feature flags, change sensitivity, report/batch intervals,
  sensor-specific config) and sends it via `bno085_send_packet()` on
  `BNO085_CHANNEL_CONTROL`. Unlike Get Feature, no response is required by
  the SH-2 spec (Get Feature Response is only sent unsolicited on a rate
  change), so this function returns as soon as the send succeeds - it does
  not loop reading responses.
- **Reuse `cmd_buf`/`cmd_len`/`bno085_read_response()` for input reports**:
  rather than adding a second receive buffer, `bno085_read_rotation_vector()`
  loops `bno085_read_response()` like `bno085_get_feature()` does, checking
  `cmd_buf[2]` (the SHTP channel of the received packet) for
  `BNO085_CHANNEL_INPUT_REPORTS` and `cmd_buf[4]`/`cmd_buf[9]` for the
  Base Timestamp Reference (0xFB) + Rotation Vector (0x05) report sequence.
  This keeps the new code consistent with the existing discard-and-retry
  pattern and avoids growing `bno085_t`'s footprint.
- **Store both raw and converted values**: `bno085_rotation_vector_t` keeps
  the raw `int16_t` Q14/Q12 fields (for exact reproduction/debugging) plus
  `float` fields converted by dividing by `1<<14` / `1<<12`, since callers
  will likely want radians/unit-quaternion values directly.
- **New channel constant**: `BNO085_CHANNEL_INPUT_REPORTS = 3`, alongside the
  existing `BNO085_CHANNEL_CONTROL = 2`.

## Risks / Trade-offs

- [Risk] A batch could contain a Timestamp Rebase (0xFA) record before the
  Base Timestamp Reference, or multiple sensor reports per packet, which
  this change does not parse. → Mitigation: such packets are simply treated
  as non-matching and discarded within the existing retry loop; this can be
  extended later if other sensors are added.
- [Risk] `BNO085_GET_FEATURE_MAX_ATTEMPTS` may be too low if the control
  channel is busy with other traffic when polling for input reports.
  → Mitigation: reuse the same constant for now (consistent with existing
  code); revisit if bench testing shows reads timing out.

## Open Questions

(none)
