## Why

`bno085_bringup()` currently performs a single 32-byte SPI read of the
SHTP advertisement response, but the advertisement is 276 bytes long
(`shtp_length=276`, `chan=0`, `seq=0` observed on the bench). The first
32 bytes are just the header plus the start of the first advertised
SHTP channel/app record. To see what SHTP channels (e.g. `control`,
`executable`, `sensorhub`, input channels) the BNO085 advertises, we
need to read the full 276-byte advertisement and parse its TLV
(tag/length/value) records.

Note: the SHTP advertisement enumerates **SHTP channels/apps**, not
physical IMU sensors (accelerometer, gyroscope, etc.) - those are
discovered via separate Product ID / Get Feature requests on the
control channel, which is out of scope for this change.

## What Changes

- Add `bno085_read_advertisement()`: performs the existing reset +
  wait-for-INT sequence, then a single larger SPI read sized to cover
  the full 276-byte advertisement.
- Add a TLV parser that walks the advertisement payload (after the
  4-byte SHTP header) as repeated `tag`/`length`/`value` records, and
  prints each record (known tags decoded - e.g. channel name strings
  and channel numbers - unknown tags printed as raw tag/length/hex).
- Update the `app_init()` demo to call the new function and print the
  parsed advertisement records over USART3 in place of (or alongside)
  the existing 32-byte bring-up dump.

## Capabilities

### New Capabilities
- `bno085-shtp-advertisement`: reading the full SHTP advertisement
  packet from the BNO085 and parsing its TLV records to enumerate
  advertised SHTP channels/apps.

### Modified Capabilities
- (none - `bno085-spi-bringup` requirements are unchanged; this adds a
  new read/parse capability built on top of the existing bring-up
  sequence)

## Impact

- `Core/Inc/bno085.h` / `Core/Src/bno085.c`: new buffer size define,
  new struct fields for the advertisement buffer and parsed channel
  table, new functions.
- `Core/Src/app.c`: demo updated to call the new function and print
  parsed advertisement records.
