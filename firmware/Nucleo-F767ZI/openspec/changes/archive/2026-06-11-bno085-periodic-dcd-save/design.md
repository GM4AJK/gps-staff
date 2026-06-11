## Context

`bno085_send_command()` already builds and sends the generic 12-byte SH-2
Command Request (0xF2) payload `{0xF2, cmd_seq, command, P0..P8}` on
`BNO085_CHANNEL_CONTROL`. `bno085_save_dcd()` and
`bno085_set_me_calibration()` both follow the pattern: send the command,
then call `bno085_read_command_response()` to wait for and parse the
matching Command Response (0xF1).

Per SH-2 Reference Manual section 6.4.7, Configure Periodic DCD Save
(0x09) is different: "There is no response to this command." The command
takes a single parameter byte P0 (`0x00` = enable, `0x01` = disable
periodic DCD save); P1..P8 are reserved.

## Goals / Non-Goals

**Goals:**
- Provide `bno085_set_periodic_dcd_save()` to enable/disable periodic DCD
  save, following the existing `bno085_send_command()` pattern.
- Enable periodic DCD save at startup in `app_init()`, alongside the
  existing ME calibration enable, as part of the forum-suggested
  calibration recipe.

**Non-Goals:**
- Not attempting the "Simple Calibration" (turntable, 0x0C) command in
  this change - that requires a structured 180-degree motion sequence
  that's impractical with the current breadboard bench setup.
- Not adding a way to read back whether periodic DCD save is enabled
  (the SH-2 manual gives no response/query for this command).

## Decisions

- **No response read**: unlike `bno085_save_dcd()` and
  `bno085_set_me_calibration()`, `bno085_set_periodic_dcd_save()` does
  not call `bno085_read_command_response()` - per the manual, the device
  sends nothing back. The function returns as soon as
  `bno085_send_command()` succeeds.
- **Function signature**: `bno085_set_periodic_dcd_save(bno085_t *p,
  uint8_t enable)` mirrors `bno085_set_me_calibration()`'s style of
  taking plain enable flags. `enable` is mapped to P0 as `enable ? 0x00 :
  0x01` (note the inverted sense vs. a typical "1 = on" flag, matching
  the SH-2 command encoding directly, documented in the Doxygen comment).
- **Wiring**: `app_init()` calls `bno085_set_periodic_dcd_save(&bno, 1)`
  right after the existing `bno085_set_me_calibration()` call, printing
  `bno085_set_periodic_dcd_save OK` or the failure status over USART3,
  matching the style of the other `app_init()` feature-enable blocks.

## Risks / Trade-offs

- [No response means a failed/ignored command is indistinguishable from
  success at the protocol level] -> Mitigation: `bno085_send_command()`'s
  return value still confirms the SPI transfer succeeded; deeper
  verification (e.g. confirming calibration data was actually persisted)
  is out of scope and would require power-cycling and re-checking
  `bno085_get_me_calibration()` status, which can be a manual bench step.
- [This may not resolve the underlying stuck `status=0` issue] ->
  Mitigation: this is an incremental diagnostic step per the forum
  recipe; if it doesn't help, the finding will be documented (as with
  prior changes) and further options (e.g. Simple Calibration 0x0C)
  considered separately.
