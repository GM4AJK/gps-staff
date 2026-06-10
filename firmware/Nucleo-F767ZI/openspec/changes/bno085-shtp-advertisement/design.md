## Context

`bno085_bringup()` (from `bno085-spi-bringup`) drives `RST` low/high,
waits for `INT` to go low, then performs a single 32-byte
`HAL_SPI_TransmitReceive()` and parses the first 4 bytes as the SHTP
header. On the bench this returned `len=276 chan=0 seq=0`, with the
remaining 244 bytes of the advertisement payload not read.

The advertisement payload (after the 4-byte SHTP header) is a sequence
of TLV records. Per the SH-2 reference advertisement format, each
record is `[tag:1][length:1][value: length bytes]`, with `tag == 0`
(`TAG_GUID`) marking the start of a new application/channel record
group. Within a group, tags such as `TAG_CHANNEL_NAME` /
`TAG_APP_NAME` carry ASCII strings (e.g. `"control"`, `"executable"`,
`"sensorhub"`) and `TAG_NORMAL_CHANNEL` / `TAG_WAKE_CHANNEL` carry the
1-byte channel number.

We only have a 32-byte capture so far, so the exact tag values cannot
be fully confirmed against the live device until the full 276-byte
advertisement is read on the bench.

## Goals / Non-Goals

**Goals:**
- Read the full 276-byte SHTP advertisement in a single bring-up pass
  (one reset/INT-wait cycle, one larger SPI transfer).
- Walk the advertisement payload as TLV records and print each record
  over USART3: tag, length, and value (decoded as an ASCII string when
  printable, a 1-byte number when length == 1, otherwise hex).
- Identify and print the advertised channel/app names and channel
  numbers, so the available SHTP channels can be enumerated.

**Non-Goals:**
- No enumeration of physical IMU sensors (accelerometer, gyro, etc.) -
  that requires Product ID / Get Feature requests on the control
  channel, a separate future change.
- No persistent storage of the parsed channel table for later use by
  other code - this change is read-and-print only.
- No support for advertisements that exceed the new fixed buffer size
  (see Decisions) - if the device ever advertises more than that, the
  read/parse is simply truncated.

## Decisions

- **Single enlarged SPI read**: define `BNO085_ADVERT_BUF_SIZE = 320`
  (covers the observed 276-byte advertisement with margin) and perform
  one `HAL_SPI_TransmitReceive()` of this size after `INT` goes low,
  reusing the existing reset-and-wait sequence from
  `bno085_bringup()`. Considered a two-phase read (read 4-byte header
  first, then read `shtp_length - 4` more bytes while `CS` stays low)
  - rejected for now as unnecessary complexity given 320 bytes is a
    trivial allocation on the F767 and a single transfer is simpler to
    reason about and debug on the scope.
- **New buffer/state separate from the bring-up `rx_buf`**: add
  `advert_buf[BNO085_ADVERT_BUF_SIZE]` and `advert_len` (the
  `shtp_length` from this read, capped to `BNO085_ADVERT_BUF_SIZE`) to
  `bno085_t`, rather than resizing the existing 32-byte `rx_buf` used
  by `bno085_bringup()`. Keeps the original bring-up behaviour/spec
  unchanged and the new buffer's purpose explicit.
- **TLV parser prints as it goes, no stored table**: implement
  `bno085_print_advertisement()` (or similar) that walks `advert_buf`
  starting at offset 4 (after the SHTP header) for `advert_len - 4`
  bytes, reading `tag`/`length`/`value` triples and transmitting a
  line per record over USART3 directly. Decoding rules, in order:
  - `length == 0`: print tag only (record separator / `TAG_GUID`
    marking a new channel group).
  - value bytes are all printable ASCII: print as a quoted string
    (covers `TAG_APP_NAME`/`TAG_CHANNEL_NAME`).
  - `length == 1`: print as a decimal number (covers channel number
    tags).
  - otherwise: print as hex bytes.
  This avoids committing to exact tag-ID-to-meaning mappings before
  bench data confirms them, while still surfacing every record
  (including the channel name strings the user is after) for visual
  inspection over the UART console.
- **Stop conditions**: the parse loop stops when the next `tag`/`length`
  pair would read past `advert_len`, or past `BNO085_ADVERT_BUF_SIZE`,
  whichever is smaller - guards against a malformed/truncated capture
  looping or reading out of bounds.
- **Debounce `INT` after releasing `RST`**: in the shared
  `bno085_reset_and_wait()` helper (factored out of `bno085_bringup()`
  for reuse by `bno085_read_advertisement()`), after driving `RST` high,
  first wait (with a short timeout, `BNO085_INT_DEASSERT_TIMEOUT_MS` =
  50ms) for `INT` to read high (deasserted) before starting the existing
  poll-for-low loop. Bench testing showed an occasional `wait=0ms`
  reading - `INT` already low immediately after releasing `RST` -
  which produced an all-`0xFF` SPI read (chip still in reset/boot, not
  yet driving `SDO`). Requiring an observed high->low transition rejects
  this spurious immediate-low reading. If `INT` is not observed high
  within `BNO085_INT_DEASSERT_TIMEOUT_MS`, proceed to the poll-for-low
  loop anyway (treat as already-deasserted) so a chip that deasserts
  `INT` faster than we can observe doesn't cause a spurious timeout.

## Risks / Trade-offs

- [Exact TAG_* numeric values are unconfirmed against this device] →
  the parser does not hardcode tag-name lookups; it prints
  tag/length/value generically with the heuristics above, so the
  output remains useful even if specific tag IDs differ from the SH-2
  reference. Tag-name lookups can be added in a follow-up once
  confirmed against the full bench dump.
- [276 bytes is close to `BNO085_ADVERT_BUF_SIZE = 320`; a future
  firmware revision could advertise more] → if `shtp_length` exceeds
  `BNO085_ADVERT_BUF_SIZE`, `advert_len` is capped and the parse simply
  covers what was captured; this is acceptable for a bring-up/inspection
  tool.
- [Single large SPI transfer assumes the BNO085 will clock out the full
  276-byte response in one continuous transaction] → if the device
  instead requires the continuation-bit/multi-packet read scheme for
  payloads beyond a smaller max-transfer size, the captured buffer
  beyond that point may be garbage; this will be visible in the bench
  hex dump and can be addressed in a follow-up if needed.
