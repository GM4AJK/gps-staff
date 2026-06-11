## Context

The driver already implements the same pattern for the Rotation Vector
(0x05) report (`bno085-rotation-vector-report`):
`bno085_set_feature()` enables a report at a given interval,
`bno085_read_rotation_vector()` reads input reports via
`bno085_read_response()` and matches on a Base Timestamp Reference
(0xFB, §7.2.1, 5 bytes) followed by the Rotation Vector report (0x05),
and `bno085_print_rotation_vector()` prints the parsed fields using the
fixed-point `bno085_format_q()` helper (newlib-nano lacks `%f`
support).

The Magnetic Field Calibrated (0x03) input report (§6.5.16.2) has the
same shape: `{Report ID, sequence, status, delay, X LSB/MSB, Y LSB/MSB,
Z LSB/MSB}` - 10 bytes, Q point 4, units uTesla. Preceded by the same
5-byte Base Timestamp Reference, the full SHTP packet is `4 (header) +
5 (base timestamp) + 10 (mag field) = 19` bytes, with the report ID
byte at `cmd_buf[9] == 0x03`.

## Goals / Non-Goals

**Goals:**
- Enable and read the Magnetic Field Calibrated report alongside the
  existing Rotation Vector report, using the same driver patterns.
- Print field magnitude (x/y/z in uT) and `status` so it can be
  compared against the phone-magnetometer baseline (~25-65 uT, "High"
  accuracy) and against the Rotation Vector's `status`/`accuracy`.

**Non-Goals:**
- No new SH-2 command plumbing - this reuses
  `bno085_set_feature()`/`bno085_read_response()` exactly as-is.
- No automatic interpretation/diagnosis in firmware (e.g. flagging
  "interference detected") - this is a manual bench diagnostic, the
  human reads the printed values.
- Magnetic Field Uncalibrated (0x0F) is out of scope.

## Decisions

- **Mirror the Rotation Vector struct/function shapes exactly**: add
  `bno085_magnetic_field_t` (`sequence`, `status`, `x`/`y`/`z` as
  `int16_t`, plus `x_f`/`y_f`/`z_f` floats) and a `magnetic_field`
  field on `bno085_t`, matching `bno085_rotation_vector_t`'s layout and
  naming conventions.
- **Reuse `bno085_format_q()`** for printing x/y/z (Q point 4) - no new
  formatting helper needed.
- **Single combined Q point constant** `BNO085_MAGNETIC_FIELD_Q_POINT`
  (4), matching the naming of `BNO085_ROTATION_VECTOR_Q_POINT`.
- **Separate `bno085_set_feature()` call** for report ID 0x03 in
  `app_init()`, same 50000us (20Hz) interval as Rotation Vector, added
  immediately after the existing Rotation Vector Set Feature call.
- **Print format**: `mag: x=<uT> y=<uT> z=<uT> seq=<n> status=<0-3>\r\n`,
  matching the `rv: ...` line's style, printed directly after the `rv:`
  line in `app_loop()`.

## Risks / Trade-offs

- [Risk] Enabling a second 20Hz report doubles the SPI/INT traffic the
  driver must service in `app_loop()`, which already reads Rotation
  Vector every iteration → Mitigation: both reads happen in the same
  500ms loop iteration as before; if INT/SPI throughput becomes an
  issue this would surface as `HAL_ERROR`/`HAL_TIMEOUT` returns, which
  are already handled by skipping the print for that iteration.
- [Risk] The diagnostic may show the magnetometer itself is fine
  (status reaching 2/3, plausible field magnitude) while Rotation
  Vector's status/accuracy remains stuck - in which case the root cause
  lies elsewhere (e.g. accel/gyro calibration, or a fusion-level issue)
  → Mitigation: this change's purpose is exactly to narrow down which
  case applies; either outcome is a useful result and will inform the
  next follow-up.
