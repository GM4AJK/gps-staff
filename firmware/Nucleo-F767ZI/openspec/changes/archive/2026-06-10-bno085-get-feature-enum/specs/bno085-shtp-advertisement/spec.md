## MODIFIED Requirements

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
