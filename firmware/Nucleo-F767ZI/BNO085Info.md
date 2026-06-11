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

## Next steps

- Bench-test the Simple Calibration "dance": press B1 (Start
  Calibration), rotate the board ~180 degrees, press B1 again (Finish
  Calibration, check finish_status == 0 for success), then power-cycle
  the board and observe whether `rv: status` / `mag: status` unstick.
