## Context

Per the SH-2 Reference Manual (1000-3625 v1.9):

- **Command Request (0xF2)**, section 6.3.8: a 12-byte control-channel
  payload - `{0xF2, sequence, command, P0..P8}`. `sequence` is a
  host-tracked, monotonically incrementing `uint8_t` used to match the
  response.
- **Command Response (0xF1)**, section 6.3.9: a 16-byte control-channel
  payload - `{0xF1, sequence, command, command_seq, response_seq,
  R0..R10}`. `command` identifies which command this responds to (with bit 7
  set for unsolicited responses), and `command_seq` echoes the `sequence`
  byte from the original Command Request.
- **Configure ME Calibration (0x07, subcommand 0x00)**, section 6.4.6.1:
  `P0..P2` = accel/gyro/mag cal enable, `P3 = 0x00`, `P4` = planar accel cal
  enable, `P5` = on-table cal enable, `P6..P8` reserved. Response `R0` =
  status (0 = success), `R1..R5` echo the enable states.
- **Get ME Calibration (0x07, subcommand 0x01)**, section 6.4.6.2: `P3 =
  0x01`, all other `P` reserved/0. Response shape matches Configure ME
  Calibration's.
- **Save DCD (0x06)**, section 6.4.5: all `P` reserved. Response `R0` =
  status (0 = success), `R1..R10` reserved.

These all fit the existing `bno085_send_packet()` /
`bno085_read_response()` plumbing on `BNO085_CHANNEL_CONTROL`: a Command
Request is 12 bytes (well under `BNO085_CMD_BUF_SIZE`), and a Command
Response packet is `4 + 16 = 20` bytes (also within `BNO085_CMD_BUF_SIZE`,
32).

## Goals / Non-Goals

**Goals:**
- Provide a small, reusable Command Request/Response helper pair, since
  Get ME Calibration, Configure ME Calibration, and Save DCD (and likely
  future commands) all share the same framing and response-matching logic.
- Enable accel/gyro/mag ME calibration after Rotation Vector is enabled, so
  `status`/`accuracy` in the Rotation Vector report can improve over time.

**Non-Goals:**
- Periodic/automatic Save DCD (Configure Periodic DCD Save, 0x09) - a single
  manual `bno085_save_dcd()` call is sufficient for now.
- Tare, simple calibration (0x0C), or other calibration-adjacent commands.
- Reading back DCD contents via FRS.

## Decisions

- **Generic command helpers**: add `bno085_send_command()`, which builds the
  12-byte `{0xF2, cmd_seq, command, P0..P8}` payload from a caller-supplied
  9-byte parameter array, sends it via `bno085_send_packet()`, records the
  sequence number used in `p->last_cmd_seq`, and increments `p->cmd_seq`
  (with `uint8_t` wraparound) only on success - mirroring how
  `bno085_send_packet()` increments `tx_seq[channel]` only on success. And
  `bno085_read_command_response()`, which loops `bno085_read_response()` up
  to `BNO085_GET_FEATURE_MAX_ATTEMPTS` times (consistent with
  `bno085_get_feature()`/`bno085_read_rotation_vector()`), matching
  `cmd_buf[4] == 0xF1 && cmd_buf[6] == command && cmd_buf[7] ==
  p->last_cmd_seq`, discarding non-matching packets. On a match, `R0..R10`
  are available at `cmd_buf[9..20)` for the caller to interpret.
- **Separate command sequence counter**: `p->cmd_seq` /
  `p->last_cmd_seq` are distinct from `tx_seq[]` (the per-SHTP-channel
  sequence numbers used by `bno085_send_packet()`'s header) - the SH-2
  Command Request/Response sequence number is a separate, command-specific
  counter per section 6.3.8.
- **`bno085_me_calibration_t`** stores `status` (from `R0`) plus
  `accel_enable`/`gyro_enable`/`mag_enable`/`planar_accel_enable`/
  `on_table_enable` (`uint8_t`, from `R1..R5`), populated by both
  `bno085_get_me_calibration()` and `bno085_set_me_calibration()` (the
  Configure response echoes the resulting enable states).
- **`bno085_save_dcd()`** only needs/exposes a status (`HAL_OK` plus
  `R0` via the shared `bno085_me_calibration_t.status` field would be
  reused - rather than adding a separate struct, the same `status` field
  is updated. `R1..R10` are reserved and not stored.

## Risks / Trade-offs

- [Risk] Enabling ME calibration alone may not be sufficient to move
  `status` off `0` quickly - the magnetometer in particular typically needs
  a figure-8 motion to converge, and there is no DCD to start from.
  → Mitigation: this change enables calibration and adds `bno085_save_dcd()`
  so that once calibration converges (through bench manipulation), it can
  be persisted; verifying convergence itself is part of bench testing, not
  a hard requirement of this change's scope.
- [Risk] `BNO085_GET_FEATURE_MAX_ATTEMPTS` may be too low if multiple
  command responses or sensor reports are queued ahead of the one we want.
  → Mitigation: reuse the existing constant for consistency; revisit if
  bench testing shows reads timing out.

## Open Questions

(none)
