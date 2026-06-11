## 1. Constants and declaration

- [x] 1.1 Add `BNO085_COMMAND_DCD_PERIODIC_SAVE` (`0x09`) constant to
      `bno085.h`, alongside `BNO085_COMMAND_SAVE_DCD` and
      `BNO085_COMMAND_ME_CALIBRATION`.
- [x] 1.2 Declare `bno085_set_periodic_dcd_save(bno085_t *p, uint8_t
      enable)` in `bno085.h` with a Doxygen comment documenting the
      `{enable ? 0x00 : 0x01, ...}` parameter mapping and the no-response
      behaviour per SH-2 Reference Manual section 6.4.7.

## 2. Implementation

- [x] 2.1 Implement `bno085_set_periodic_dcd_save()` in `bno085.c`:
      build `params = {enable ? 0x00 : 0x01, 0,0,0,0,0,0,0,0}`, call
      `bno085_send_command(p, BNO085_COMMAND_DCD_PERIODIC_SAVE, params)`,
      and return its result directly (no
      `bno085_read_command_response()` call).

## 3. Wire-up and review

- [x] 3.1 In `Core/Src/app.c`'s `app_init()`, after the existing
      `bno085_set_me_calibration()` call, add a call to
      `bno085_set_periodic_dcd_save(&bno, 1)` and print
      `bno085_set_periodic_dcd_save OK` (or the failure status) over
      USART3, following the existing feature-enable print style.
- [x] 3.2 Bench-test on hardware: verify
      `bno085_set_periodic_dcd_save OK` is printed at startup, and
      observe whether `rv: status` / `mag: status` change behaviour
      (over a longer run, and/or after a power cycle) compared to the
      stuck `status=0` baseline from bno085-magnetic-field-report.

      Result: `bno085_set_periodic_dcd_save OK` printed at startup,
      confirming the command sends successfully. `mag: status` remained
      stuck at `0` throughout a ~30-line run (field magnitude stable at
      ~38-40uT). One `seq` discontinuity (229 -> 7) was observed
      mid-run, possibly an internal sensor hub reset, but `status`
      stayed `0` on both sides of it. Periodic DCD save alone does not
      unstick the calibration confidence classifier; left as further
      follow-up (e.g. Simple/Turntable Calibration command 0x0C, which
      requires a structured 180-degree motion).
