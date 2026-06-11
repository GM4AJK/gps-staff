## 1. Constants and struct fields

- [x] 1.1 Add `BNO085_REPORT_ID_MAGNETIC_FIELD` (`0x03`) and
      `BNO085_MAGNETIC_FIELD_Q_POINT` (`4`) constants to `bno085.h`.
- [x] 1.2 Add `bno085_magnetic_field_t` struct (`sequence`, `status`,
      `x`/`y`/`z` as `int16_t`, `x_f`/`y_f`/`z_f` as `float`) and a
      `magnetic_field` field on `bno085_t` to `bno085.h`.

## 2. Read and print

- [x] 2.1 Implement `bno085_read_magnetic_field()` in `bno085.c`,
      mirroring `bno085_read_rotation_vector()`'s structure: loop
      `bno085_read_response()`, match on `cmd_len >= 19`,
      `cmd_buf[4] == BNO085_REPORT_ID_BASE_TIMESTAMP`, `cmd_buf[9] ==
      BNO085_REPORT_ID_MAGNETIC_FIELD`, then parse `cmd_buf[10..19)`.
- [x] 2.2 Implement `bno085_print_magnetic_field()` in `bno085.c`,
      mirroring `bno085_print_rotation_vector()` and reusing
      `bno085_format_q()`.
- [x] 2.3 Document both functions in `bno085.h` following the existing
      Doxygen-style comment conventions.

## 3. Wire-up and review

- [x] 3.1 In `Core/Src/app.c`'s `app_init()`, after the existing
      Rotation Vector `bno085_set_feature()` call, add a
      `bno085_set_feature()` call enabling report `0x03` at the same
      50000us interval, and print its result.
- [x] 3.2 In `Core/Src/app.c`'s `app_loop()`, call
      `bno085_read_magnetic_field()` and, on `HAL_OK`, call
      `bno085_print_magnetic_field()` alongside the existing Rotation
      Vector read/print.
- [x] 3.3 Bench-test on hardware: verify `mag: x=... y=... z=...
      seq=... status=...` lines are printed, that the field magnitude
      is plausible (~25-65 uT for Earth's field, comparable to the
      phone-magnetometer baseline), and observe whether `status`
      changes with motion (compare against the Rotation Vector's
      `status`/`acc`, which remained stuck at `0`/`pi`).

      Result: `mag:` lines print correctly; field magnitude settled
      around ~40-45uT (e.g. x=-41.6 y=1.8 z=-15.8), consistent with
      Earth's field and the phone-magnetometer baseline (~45-50uT).
      However `mag: status` is also stuck at `0`, exactly mirroring
      `rv: status`. Cross-checked our byte offsets/Q points and the
      ME calibration enable call against Adafruit's
      `sh2_SensorValue.c`/`sh2.c` (Adafruit_BNO08x library) — both
      match exactly, so this isn't a driver bug. Since the readings
      themselves are physically plausible, the stuck status=0 looks
      like a calibration-confidence-classifier issue rather than bad
      magnetometer data; left as a further follow-up investigation.
