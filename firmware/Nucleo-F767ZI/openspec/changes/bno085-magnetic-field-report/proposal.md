## Why

The `bno085-me-calibration` change enabled accel/gyro/mag dynamic
calibration and confirmed (via `bno085_get_me_calibration()`) that the
enables persist on-chip. Despite this, and despite ~1 minute of varied
3D motion on the bench, the Rotation Vector (0x05) report's `status`
and `accuracy` fields stayed pinned at `0` ("Unreliable") and `pi`
radians (the worst-case/default value) throughout - never fluctuating
even briefly, which is more "stuck" than reports of bad setups in the
community (where `status` is typically observed to fluctuate between
0-3 with motion).

A community thread (Adafruit forums, "BNO085 calibration and
performance") repeatedly points to the magnetometer as the limiting
factor for Rotation Vector accuracy, and recommends independently
reading the Magnetic Field Calibrated (0x03) report - both its field
magnitude (Earth's field is ~25-65 uT) and its own `status` byte - to
diagnose whether the issue is magnetometer-specific (e.g. local
magnetic interference from nearby ferrous/metal components) versus a
broader fusion/calibration problem.

This change adds that diagnostic: reading and printing the Magnetic
Field Calibrated report alongside the existing Rotation Vector output,
so the two can be compared directly on the bench.

## What Changes

- Enable the Magnetic Field Calibrated (0x03) report via the existing
  `bno085_set_feature()` in `app_init()`, at the same 20 Hz interval
  used for Rotation Vector.
- Add `bno085_read_magnetic_field()`: reads an SH-2 Magnetic Field
  Calibrated (0x03) input report (preceded by a Base Timestamp
  Reference, as with Rotation Vector) and parses `status` and the
  x/y/z field components (Q point 4, units uT) into a new
  `bno085_magnetic_field_t` struct on `bno085_t`.
- Add `bno085_print_magnetic_field()`: prints the parsed status and
  x/y/z field values (in uT, to 4 decimal places using the existing
  `bno085_format_q()`-style fixed-point formatting) over USART3.
- Wire `bno085_read_magnetic_field()` /
  `bno085_print_magnetic_field()` into `app_loop()` alongside the
  existing Rotation Vector read/print.

## Capabilities

### New Capabilities
- `bno085-magnetic-field-report`: enabling, reading, and printing the
  SH-2 Magnetic Field Calibrated (0x03) input report.

### Modified Capabilities
- (none)

## Impact

- `Core/Inc/bno085.h` / `Core/Src/bno085.c`: new
  `BNO085_REPORT_ID_MAGNETIC_FIELD` constant, new
  `bno085_magnetic_field_t` struct and `magnetic_field` field on
  `bno085_t`, new `bno085_read_magnetic_field()` and
  `bno085_print_magnetic_field()` functions.
- `Core/Src/app.c`: `app_init()` enables the Magnetic Field Calibrated
  report; `app_loop()` reads and prints it alongside Rotation Vector.
