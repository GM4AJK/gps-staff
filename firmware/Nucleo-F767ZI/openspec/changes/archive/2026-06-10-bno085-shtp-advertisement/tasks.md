## 1. Driver: full advertisement read

- [x] 1.1 In `Core/Inc/bno085.h`, add `#define BNO085_ADVERT_BUF_SIZE 320`
      and add `advert_buf[BNO085_ADVERT_BUF_SIZE]` and `uint16_t
      advert_len` fields to `bno085_t`
- [x] 1.2 In `Core/Src/bno085.c`, factor the existing reset-pulse +
      wait-for-`INT` sequence out of `bno085_bringup()` into a static
      helper, and implement `bno085_read_advertisement()`: run the
      helper, then perform a single `BNO085_ADVERT_BUF_SIZE`-byte
      `HAL_SPI_TransmitReceive()` with `CS` asserted (all-zero TX),
      release `CS`, parse the first 4 bytes as the SHTP header, and
      store `advert_len = min(shtp_length, BNO085_ADVERT_BUF_SIZE)`
- [x] 1.3 Declare `bno085_read_advertisement()` in `Core/Inc/bno085.h`

## 2. Driver: TLV parser

- [x] 2.1 Implement `bno085_print_advertisement()` in
      `Core/Src/bno085.c`: walk `advert_buf[4..advert_len)` as
      `tag`/`length`/`value` records, transmitting one line per record
      over a given `UART_HandleTypeDef *` per the encoding rules in
      design.md (empty, quoted ASCII string, decimal byte, or hex
      bytes), stopping before any record that would read past
      `advert_len` or `BNO085_ADVERT_BUF_SIZE`
- [x] 2.2 Declare `bno085_print_advertisement()` in `Core/Inc/bno085.h`

## 3. Sandbox integration

- [x] 3.1 In `Core/Src/app.c`, after the existing `bno085_bringup()`
      demo, call `bno085_read_advertisement()` and report its HAL
      status over USART3, then on success call
      `bno085_print_advertisement()` to print the parsed records

## 4. Bench verification

- [x] 4.1 Build and flash via the external ST-LINK V3, confirm
      `bno085_read_advertisement()` returns `HAL_OK` with `advert_len`
      around 276, and that `bno085_print_advertisement()` prints a
      plausible set of records (including readable channel/app name
      strings) over USART3
