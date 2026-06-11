## 1. Constants and struct fields

- [x] 1.1 Add `BNO085_REPORT_ID_COMMAND_REQUEST` (`0xF2`),
      `BNO085_REPORT_ID_COMMAND_RESPONSE` (`0xF1`),
      `BNO085_COMMAND_ME_CALIBRATION` (`0x07`), and
      `BNO085_COMMAND_SAVE_DCD` (`0x06`) constants to `bno085.h`.
- [x] 1.2 Add `bno085_me_calibration_t` struct (`status`, `accel_enable`,
      `gyro_enable`, `mag_enable`, `planar_accel_enable`,
      `on_table_enable`, all `uint8_t`) to `bno085.h`.
- [x] 1.3 Add `cmd_seq`, `last_cmd_seq` (`uint8_t`), and a `me_calibration`
      field of type `bno085_me_calibration_t` to `bno085_t`.

## 2. Command Request/Response helpers

- [x] 2.1 Implement `bno085_send_command()` in `bno085.c`: build the 12-byte
      Command Request payload from `command` and a 9-byte parameter array,
      send via `bno085_send_packet()`, and update `cmd_seq`/`last_cmd_seq`
      on success.
- [x] 2.2 Implement `bno085_read_command_response()` in `bno085.c`: loop
      `bno085_read_response()` up to `BNO085_GET_FEATURE_MAX_ATTEMPTS`
      times, matching on report ID/command/sequence as specified.
- [x] 2.3 Document both functions in `bno085.h` following the existing
      Doxygen-style comment conventions.

## 3. ME Calibration commands

- [x] 3.1 Implement `bno085_get_me_calibration()` in `bno085.c`.
- [x] 3.2 Implement `bno085_set_me_calibration()` in `bno085.c`.
- [x] 3.3 Implement `bno085_save_dcd()` in `bno085.c`.
- [x] 3.4 Document all three functions in `bno085.h`.

## 4. Wire-up and review

- [x] 4.1 In `Core/Src/app.c`, after enabling the Rotation Vector report,
      call `bno085_set_me_calibration()` to enable accel/gyro/mag
      calibration (planar/on-table disabled), and print the result.
- [x] 4.2 Bench-test on hardware: verify Get/Configure ME Calibration and
      Save DCD commands get matching responses, and observe whether
      Rotation Vector `status`/`accuracy` improve as the board is moved
      (e.g. figure-8 motion for magnetometer convergence).

      Result: `bno085_set_me_calibration(&bno, 1, 1, 1, 0, 0)` returns
      `HAL_OK` with `status=0`, and a periodic `bno085_get_me_calibration()`
      readback confirms `accel_enable=1 gyro_enable=1 mag_enable=1` persists
      on-chip — the Command Request/Response plumbing and ME Calibration
      commands work as specified. Rotation Vector `status`/`acc` remained at
      `0`/`pi` through ~1 minute of varied 3D motion; the SH-2 manual does
      not document the fusion confidence-estimation algorithm's convergence
      behaviour, so improving `status`/`acc` is left as a follow-up
      investigation.
