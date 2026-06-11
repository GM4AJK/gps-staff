## Why

The bno085-magnetic-field-report change confirmed that both the Rotation
Vector's and Magnetic Field Calibrated report's `status` fields remain
stuck at `0` (Unreliable), despite plausible sensor readings and motion,
with ME calibration enabled via `bno085_set_me_calibration()`. Community
research (Adafruit BNO085 forum thread on calibration/performance) points
to ME calibration enable plus Configure Periodic DCD Save (0x09) together
as a working recipe for getting the calibration accuracy/status classifier
to progress. We want to add the periodic DCD save command as the next
diagnostic step.

## What Changes

- Add `BNO085_COMMAND_DCD_PERIODIC_SAVE` (`0x09`) constant to `bno085.h`.
- Add `bno085_set_periodic_dcd_save(bno085_t *p, uint8_t enable)` to
  `bno085.c`, sending a Configure Periodic DCD Save command via
  `bno085_send_command()`. Per SH-2 Reference Manual section 6.4.7, this
  command has **no response** - `bno085_read_command_response()` is not
  called.
- Wire a call enabling periodic DCD save (`enable=1`) into `app_init()`,
  alongside the existing ME calibration enable call, printing the send
  result over USART3.

## Capabilities

### New Capabilities
- `bno085-periodic-dcd-save`: enabling the SH-2 Configure Periodic DCD
  Save (0x09) command to automatically persist dynamic calibration data.

### Modified Capabilities
(none)

## Impact

- `Core/Inc/bno085.h`: new constant and function declaration.
- `Core/Src/bno085.c`: new `bno085_set_periodic_dcd_save()` function.
- `Core/Src/app.c`: `app_init()` enables periodic DCD save at startup.
