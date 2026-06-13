# BNO085 - Closing Notes

Wrap-up for the BNO085/SH-2 exploration across two sandbox boards. The
chip will not be carried forward to the GPS staff design - this file
records why, for future reference if a BNO08x part is ever reconsidered.

## Phase 1: SPI/SHTP transport (Nucleo-F767ZI, archived)

Initial driver work targeted the SPI interface. The SHTP/SPI transport
proved poor and over-complex to work with reliably - wake/INT
handshaking and response framing were fragile, and a Simple/Turntable
Calibration command (0x0C) consistently failed in ways that didn't
match the documented protocol. This work was parked and later removed
from `main` (PR #86); history remains in archived branches
(`bno085-simple-calibration`, `bno085-sh2`, PRs #84/#85).

## Phase 2: I2C/SH-2 side project (Nucleo-F446RE, this branch)

Switched to I2C (address 0x4A, shared bus with the SSD1309 OLED at
0x3C) to rule out SPI-specific issues. Progress was much better here:

- Rotation Vector (0x05) decodes cleanly into roll/pitch/yaw, a
  0-360 degree bearing (with an empirical 180-degree correction for
  this board's silkscreen orientation) and a tilt-from-vertical angle.
  This part of the chip works well.
- Magnetic Field Calibrated (0x03) total field strength tracked
  Earth's field plausibly (~26-45uT depending on location/handling).

But once the transport-level issues were out of the way, the more
fundamental problem reappeared at the device level:

- The Rotation Vector and Magnetic Field status/accuracy fields stayed
  pinned at 0/3 (Unreliable) regardless of ME Calibration (0xF2/0x07)
  being enabled and confirmed, and despite extensive figure-8 and 3D
  motion. This matches the same "status stuck at 0" symptom seen on
  the SPI/F767ZI driver, on different hardware and a different
  transport - so it's a characteristic of the chip/firmware, not a
  driver bug.
- Forum research (SparkFun, Adafruit, PJRC, SlimeVR) shows other users
  report similar opaque/ineffective magnetometer calibration on both
  BNO085 and its successor BNO086, plus drift over repeated rotations
  and deviation near magnetic interference.
- Hillcrest's own BNO080 Sensor Calibration Procedure (doc 1000-4044)
  describes a specific structured motion sequence (discrete 4-6 cube
  orientations for accel, a still period for gyro, then individual
  ~180-degree roll/pitch/yaw sweeps one axis at a time for mag) that
  was never tried here - our undirected figure-8 motion may simply not
  match what the fusion algorithm expects.

### Exec time / clock-stretching finding

With the Rotation Vector report enabled at its 100ms (10Hz) interval
and `I2C1.NoStretchMode = DISABLE`, `app_loop()`'s average exec time
for `test_bno085_rotation_vector_display()` sat at ~85-105ms per call -
essentially the full report period. `bno085_read_packet()`'s
`HAL_I2C_Master_Receive()` blocks via I2C clock stretching until the
chip has a new report ready, so the loop timing is dictated entirely by
the BNO085's internal report scheduling, not by anything in our code.

This is one more example of running against an opaque "black box":
even basic loop timing on our side is governed by undocumented internal
behaviour of the sensor hub firmware, with no visibility into why.

## Conclusion (2026-06-13)

A worthwhile evening's exploration to better understand the chip, but
between the fragile SPI transport, the persistently-stuck mag
calibration across two independent implementations, and timing
governed by an opaque internal scheduler, the BNO08x series isn't
trusted as the primary heading source for this project.
**LSM6DSOX + LIS3MDL remains the spec'd tilt+azimuth solution** (see
`sdd/README.md`). The BNO085 footprint will *not* be added to the final
PCB, to avoid the temptation to revisit this.
