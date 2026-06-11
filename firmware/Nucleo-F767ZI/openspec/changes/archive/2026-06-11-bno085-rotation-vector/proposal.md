## Why

The BNO085 driver can currently query a sensor's existing configuration
(`bno085_get_feature()`) but cannot enable a sensor report, and has no way to
read or parse the resulting sensor input reports. To get orientation data out
of the chip, the host must send a Set Feature Command (0xFD) to enable the
Rotation Vector report (0x05) at a chosen interval, then read and decode the
resulting input reports (Base Timestamp Reference + Rotation Vector) arriving
on the SHTP input-reports channel.

## What Changes

- Add `bno085_set_feature()`, which sends an SH-2 Set Feature Command (0xFD)
  on the SH-2 control channel via `bno085_send_packet()`, with caller-supplied
  feature flags, change sensitivity, report interval, batch interval, and
  sensor-specific configuration.
- Add `bno085_read_input_report()`, which reads a packet from the SHTP
  input-reports channel (channel 3) using the same exact-length two-step read
  as `bno085_read_response()`/`bno085_read_advertisement()`.
- Add parsing of the Rotation Vector (0x05) input report - including a
  preceding Base Timestamp Reference (0xFB) record - storing the quaternion
  (i, j, k, real components, Q point 14) and heading accuracy estimate (Q
  point 12) in the `bno085_t` struct.
- Add a debug print helper for the parsed rotation vector, following the
  pattern of `bno085_print_advertisement()`.

## Capabilities

### New Capabilities
- `bno085-set-feature`: sending an SH-2 Set Feature Command (0xFD) to enable
  a sensor report at a configured interval.
- `bno085-rotation-vector-report`: reading SHTP input-report packets and
  parsing the Base Timestamp Reference (0xFB) and Rotation Vector (0x05)
  reports into the `bno085_t` struct.

### Modified Capabilities
(none)

## Impact

- `Core/Inc/bno085.h`, `Core/Src/bno085.c`: new struct fields
  (`bno085_rotation_vector_t`, input-report buffer/length), new constants
  (report IDs, channel number, Q-point shifts), and new functions
  (`bno085_set_feature()`, `bno085_read_input_report()`,
  `bno085_print_rotation_vector()`).
- No change to existing function signatures or behavior.
