# bno085-shtp-advertisement

## Purpose

Reads the full BNO085 SHTP advertisement packet over SPI and parses its
TLV records, enumerating the advertised SHTP channels/apps (e.g.
`control`, `executable`, `sensorhub`, input channels) for inspection.

## Requirements

### Requirement: Read the full SHTP advertisement
The driver SHALL provide `bno085_read_advertisement()`, which performs
the same `RST` pulse and `INT`-wait sequence as `bno085_bringup()`,
then asserts `CS`, performs a 4-byte full-duplex SPI transfer
(transmitting all-zero bytes) to read the SHTP header, computes
`advert_len = min(shtp_length, BNO085_ADVERT_BUF_SIZE)` from it, and
then performs a second full-duplex SPI transfer of exactly
`advert_len - 4` more bytes (if any) before releasing `CS` high. Only
`advert_buf[0 .. advert_len)` is written by this call; bytes beyond
`advert_len` are not written.

#### Scenario: Successful advertisement read
- **WHEN** `bno085_read_advertisement()` is called, `INT` goes low
  within `BNO085_INT_TIMEOUT_MS` of releasing `RST`, and both SPI
  transfers succeed
- **THEN** `bno085_read_advertisement()` returns `HAL_OK`
- **AND** `advert_buf[0 .. advert_len)` contains the received SHTP
  header and payload, where `advert_len` equals the SHTP header's
  length field, capped to `BNO085_ADVERT_BUF_SIZE`
- **AND** exactly `advert_len` bytes total are clocked over SPI (no
  more, no fewer), so the device's queue is not left misaligned for
  subsequent reads

#### Scenario: INT timeout returns an error without an SPI transaction
- **WHEN** `bno085_read_advertisement()` is called and `INT` does not go
  low within `BNO085_INT_TIMEOUT_MS` of releasing `RST`
- **THEN** `bno085_read_advertisement()` returns `HAL_TIMEOUT`
- **AND** no SPI transfer is performed
- **AND** `CS` remains high (deasserted)

#### Scenario: SPI failure during the read is propagated
- **WHEN** `INT` goes low within the timeout, but either the
  header-read or the payload-read `HAL_SPI_TransmitReceive()` call
  fails (error or timeout)
- **THEN** `bno085_read_advertisement()` returns the corresponding
  non-`HAL_OK` `HAL_StatusTypeDef`
- **AND** `CS` is released high (deasserted) before returning

### Requirement: Parse and print SHTP advertisement TLV records
The driver SHALL provide `bno085_print_advertisement()`, which walks
`advert_buf` starting at offset 4 (immediately after the SHTP header)
for `advert_len - 4` bytes as a sequence of `tag`/`length`/`value`
records (1 byte tag, 1 byte length, `length` bytes of value), and
transmits one line per record over the given UART. For each record:
- if `length == 0`, the line SHALL show only the tag
- a single trailing NUL byte (`0x00`), if present, SHALL be trimmed
  from `value` before the printable check and SHALL NOT be included in
  the displayed string
- if the (possibly trimmed) `value` is non-empty and consists entirely
  of printable ASCII bytes, the line SHALL show it as a quoted string
- if `length == 1` and `value` is not printable ASCII, the line SHALL
  show `value` as a decimal number
- otherwise, the line SHALL show `value` as space-separated hex bytes

The walk SHALL stop before reading a `tag`/`length`/`value` record that
would extend past `advert_len` or past `BNO085_ADVERT_BUF_SIZE`,
whichever is smaller.

#### Scenario: Advertisement records are printed
- **WHEN** `bno085_print_advertisement()` is called after a successful
  `bno085_read_advertisement()`
- **THEN** one line is transmitted over USART3 for each complete
  `tag`/`length`/`value` record found in `advert_buf[4..advert_len)`
- **AND** records whose value is printable ASCII are shown as quoted
  strings

#### Scenario: Truncated trailing record is not printed
- **WHEN** the bytes remaining in `advert_buf[4..advert_len)` are fewer
  than 2 (a tag and length byte) or fewer than `2 + length` (the full
  value)
- **THEN** that trailing record is not printed
- **AND** no out-of-bounds read of `advert_buf` occurs
