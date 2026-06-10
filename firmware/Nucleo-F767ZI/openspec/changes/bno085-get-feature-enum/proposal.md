## Why

The SHTP advertisement (`bno085-shtp-advertisement`) tells us which SHTP
*channels* exist (`control`, `executable`, `device`, `sensorhub`,
`inputNormal`, `inputWake`, `inputGyroRv`) and, in its `tag 129` table,
which SH-2 *report IDs* the firmware supports (e.g. `01 0A` =
Accelerometer with a 10-byte report, `02 0A` = Gyroscope, `05 0E` =
Rotation Vector, etc.) - but it doesn't tell us which of those sensors
are currently *enabled*. The SH-2 "Get Feature Request"/"Get Feature
Response" pair (report IDs `0xFE`/`0xFC`) on the SHTP control channel
(channel 2, `sensorhub:control`) queries a sensor's current
configuration (report interval, etc.), letting us enumerate which
sensors are actually active.

This requires the driver's first SPI *write* (host -> device) -
everything so far has only been a single SPI read after `INT` goes
low.

## What Changes

- Add `bno085_send_packet()`: writes an SHTP packet (4-byte header +
  payload) to a given channel, asserting/releasing `CS` around a single
  `HAL_SPI_TransmitReceive()` (no `INT` wait - writes are host-initiated).
  Tracks a per-channel host TX sequence number in `bno085_t`.
- Add `bno085_get_feature()`: sends a Get Feature Request (`0xFE`,
  `report_id`, 4 zero bytes) on the SH-2 control channel, waits for
  `INT`, reads the response into a small command buffer, and parses a
  Get Feature Response (`0xFC`) into a `bno085_feature_t` struct
  (feature report ID, flags, change sensitivity, report interval,
  batch interval, sensor-specific config).
- Update the `app_init()` demo to call `bno085_get_feature()` for a
  fixed list of well-known sensor report IDs (Accelerometer, Gyroscope,
  Magnetic Field, Rotation Vector, Game Rotation Vector) and print each
  one's response (or "not supported"/timeout) over USART3.

## Capabilities

### New Capabilities
- `bno085-get-feature-enum`: sending an SHTP packet (host -> device)
  and using SH-2 Get Feature Request/Response to query and print each
  queried sensor's current configuration.

### Modified Capabilities
- (none)

## Impact

- `Core/Inc/bno085.h` / `Core/Src/bno085.c`: new per-channel TX
  sequence numbers, new command buffer, new `bno085_feature_t` struct,
  new `bno085_send_packet()` and `bno085_get_feature()` functions.
- `Core/Src/app.c`: demo updated to call `bno085_get_feature()` for a
  fixed list of sensor report IDs and print the results.
