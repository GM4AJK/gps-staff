## Why

After enabling the Rotation Vector report (`bno085-rotation-vector`), bench
testing shows `status` stuck at `0` ("Unreliable") and `accuracy` stuck at
`pi` (the maximum/placeholder uncertainty) for minutes, even with the board
moved around. Per the SH-2 Reference Manual (section 6.4.6), the
accelerometer/gyroscope/magnetometer dynamic calibration ("ME Calibration")
routines must be explicitly enabled via a Command Request - their
enabled/disabled state on a fresh reset (with no stored DCD) is not under
our control until we send this command. Without ME calibration running, the
fusion's accuracy estimate has no path to improve.

## What Changes

- Add generic SH-2 Command Request/Response (0xF2/0xF1) support: a 12-byte
  Command Request payload with a host-tracked command sequence number, sent
  via `bno085_send_packet()`, and a matching response read/parsed via
  `bno085_read_response()`.
- Add `bno085_get_me_calibration()` (Get ME Calibration, command `0x07`
  subcommand `0x01`) to query current accel/gyro/mag/planar/on-table
  calibration enable states.
- Add `bno085_set_me_calibration()` (Configure ME Calibration, command
  `0x07` subcommand `0x00`) to enable/disable each of those calibration
  routines.
- Add `bno085_save_dcd()` (Save DCD, command `0x06`) to persist calibration
  data so it survives a reset.
- Wire into `app.c`: after enabling Rotation Vector, enable accel/gyro/mag
  ME calibration (planar/on-table left disabled).

## Capabilities

### New Capabilities
- `bno085-me-calibration`: SH-2 Command Request/Response infrastructure plus
  Get/Configure ME Calibration and Save DCD commands.

### Modified Capabilities
(none)

## Impact

- `Core/Inc/bno085.h`, `Core/Src/bno085.c`: new struct fields (command
  sequence counter, `bno085_me_calibration_t`), new constants (command IDs,
  subcommands, report IDs `0xF2`/`0xF1`), and new functions
  (`bno085_get_me_calibration()`, `bno085_set_me_calibration()`,
  `bno085_save_dcd()`).
- `Core/Src/app.c`: after Rotation Vector is enabled, enable accel/gyro/mag
  ME calibration.
- No change to existing function signatures or behavior.
