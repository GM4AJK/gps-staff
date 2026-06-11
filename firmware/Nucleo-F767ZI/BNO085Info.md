# BNO085 Driver - Running Notes

Working notes for the BNO085 SH-2 driver (`Core/Inc/bno085.h`,
`Core/Src/bno085.c`) being built up on the Nucleo-F767ZI sandbox board.
This file is the running record of what's been implemented and what's
been found, used to do a single openspec spec sync once the driver
reaches a stopping point - individual steps are no longer tracked via
per-change openspec proposals.

## Capabilities implemented so far

- SPI bring-up / reset sequencing (`bno085_bringup`)
- SHTP advertisement read/parse (`bno085_read_advertisement`,
  `bno085_print_advertisement`)
- Generic SHTP packet send/receive (`bno085_send_packet`,
  `bno085_read_response`)
- Get/Set Feature (`bno085_get_feature`, `bno085_set_feature`)
- Rotation Vector (0x05) read/print (`bno085_read_rotation_vector`,
  `bno085_print_rotation_vector`)
- Magnetic Field Calibrated (0x03) read/print
  (`bno085_read_magnetic_field`, `bno085_print_magnetic_field`)
- Generic Command Request/Response (`bno085_send_command`,
  `bno085_read_command_response`)
- Configure ME Calibration (0x07) get/set
  (`bno085_get_me_calibration`, `bno085_set_me_calibration`)
- Save DCD (0x06) (`bno085_save_dcd`)
- Configure Periodic DCD Save (0x09) (`bno085_set_periodic_dcd_save`)
- Simple/Turntable Calibration (0x0C) Start/Finish
  (`bno085_start_calibration`, `bno085_finish_calibration`), driven via
  the B1 user button (PC13) as a 3-state dance in `app_loop()`:
  press 1 = Start Calibration (then rotate ~180deg), press 2 = Finish
  Calibration (then power-cycle to apply), press 3+ = no-op

## Open investigation: status stuck at 0

Both `rv: status` (Rotation Vector) and `mag: status` (Magnetic Field
Calibrated) report fields remain pinned at `0` (Unreliable), despite:
- ME calibration enabled (`accel=1 gyro=1 mag=1`)
- Plausible, stable readings (mag field ~38-45uT, consistent with
  Earth's field and a phone-magnetometer baseline ~45-50uT)
- Periodic DCD save enabled (sends OK, no observable effect)
- ~1 minute+ of motion on the bench

Driver byte offsets/Q points/calibration-enable approach were
cross-checked against Adafruit's `Adafruit_BNO08x` library
(`sh2.c`/`sh2_SensorValue.c`) and match exactly - not a driver bug.

Forum research (Adafruit BNO085 community thread, "calibration and
performance") suggests:
- Status normally fluctuates with motion/stillness in working setups
- ME cal enable + periodic DCD save is a commonly-cited recipe (tried,
  no effect so far)
- Simple/Turntable Calibration command (0x0C, SH-2 manual section
  6.4.10) is the next thing to try - requires Start Calibration ->
  180-degree motion -> Finish Calibration -> reset sensor hub for new
  calibration to take effect

## Status: PARKED (2026-06-11)

Driver development on this branch (`bno085-simple-calibration`) is
paused. Work so far is committed and pushed (PR #84), but the Simple
Calibration command does not work and the underlying cause is not
understood well enough to keep "trying things" - see below.

The user is going to read the BNO085/SH-2 datasheets in full before
continuing, either to write the SPI/SHTP transport layer themselves or
to direct further driver changes with a clearer understanding of the
expected protocol timing/behaviour. **Do not resume guessing at fixes
for this issue without that direction.**

### Simple Calibration (0x0C) bench-test results

Pressing B1 to call `bno085_start_calibration()` consistently fails
(`HAL_ERROR`). Debug instrumentation added during this investigation
(dumping `cmd_len` and the first 12 bytes of `cmd_buf` on failure)
showed:

- Every `bno085_read_response()` call inside
  `bno085_read_command_response()` returns `cmd_len == 0` (an
  all-zero 4-byte SHTP header) - i.e. `bno085_wake_and_wait_int_low()`
  reports "ready" (INT already low / flag already set) on every
  attempt, but the device has nothing queued.
- This happens even after:
  - Increasing the match-attempt budget from 4 to 16
    (`BNO085_COMMAND_RESPONSE_MAX_ATTEMPTS`).
  - Separately capping/skipping empty packets without spending the
    attempt budget, up to 64 total reads
    (`BNO085_COMMAND_RESPONSE_MAX_TOTAL_READS`).
  - Adding a 1ms `HAL_Delay()` between empty-packet retries.
- Meanwhile `bno085_read_rotation_vector()` /
  `bno085_read_magnetic_field()` continue to succeed normally in the
  same loop, both before and after the failed command.
- `bno085_get_me_calibration()` (also a Command Request/Response, 0x07)
  *did* work earlier in `app_init()`, using the same
  `bno085_send_command()` / `bno085_read_command_response()` path with
  the original budget of 4 - so the generic command path is not
  obviously broken, but something about the SHTP/SPI transport state
  around sending 0x0C and reading its response specifically is not
  understood.

None of the three changes above (16-attempt budget, empty-packet
skip+64 cap, 1ms pacing delay) fixed the symptom - this looks like a
transport/protocol-level misunderstanding (INT/WAKE handshake timing,
SHTP packet framing, or SPI CS/clock requirements between back-to-back
transactions) rather than something fixable by tuning retry counts.
All three changes are committed on this branch (PR #84) for reference
but should be reviewed/possibly reverted once the transport is better
understood, rather than built upon further.

### Open investigation: status stuck at 0

Still unresolved (see above) - both `rv: status` and `mag: status`
remain pinned at `0` (Unreliable). The Simple Calibration "dance"
(Start -> rotate ~180deg -> Finish -> power-cycle) could not be
completed because Start Calibration itself fails to get a response.

## Next steps

- User to read the SH-2 reference manual and BNO085 datasheet in full
  to build a clear model of the expected SPI/SHTP transport behaviour
  (INT/WAKE handshake, packet framing, timing between transactions),
  then either write the transport layer directly or direct specific,
  understood changes here.
- Once the transport-level understanding is in place, revisit:
  - Why `bno085_read_command_response()` sees all-empty packets after
    sending command 0x0C, when 0x07 (ME Calibration) worked.
  - The `status` stuck-at-0 root cause.
