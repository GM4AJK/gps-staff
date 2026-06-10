## 1. BNO085 driver scaffold

- [x] 1.1 Create `Core/Inc/bno085.h` with the `bno085_t` handle struct (SPI
      handle pointer, CS/RST/INT GPIO ports/pins, raw receive buffer of
      `BNO085_BRINGUP_BUF_SIZE` (32) bytes, `shtp_length`/`shtp_channel`/
      `shtp_sequence` fields), and `BNO085_RESET_PULSE_MS` (10),
      `BNO085_INT_TIMEOUT_MS` (1000), `BNO085_BRINGUP_BUF_SIZE` (32) defines
- [x] 1.2 Implement `bno085_init()` in `Core/Src/bno085.c`: populate the
      handle fields, no SPI/GPIO activity
- [x] 1.3 Implement `bno085_bringup()` in `Core/Src/bno085.c`: drive `RST`
      low for `BNO085_RESET_PULSE_MS` then high; poll `INT` for low with
      `BNO085_INT_TIMEOUT_MS` timeout; on success, assert `CS`, perform a
      `BNO085_BRINGUP_BUF_SIZE`-byte `HAL_SPI_TransmitReceive()` with
      all-zero TX data, deassert `CS`, and parse the SHTP header (length,
      channel, sequence) from the first 4 received bytes
- [x] 1.4 Declare `bno085_init()` and `bno085_bringup()` in
      `Core/Inc/bno085.h`

## 2. Sandbox integration

- [x] 2.1 In `Core/Src/app.c`, add a static `bno085_t` handle, call
      `bno085_init()` with `&hspi1` and the BNO085 CS/RST/INT GPIO
      ports/pins from `main.h`, call `bno085_bringup()` in `app_init()`, and
      report the result (HAL status, and on success the parsed
      `shtp_length`/`shtp_channel`/`shtp_sequence`) over USART3

## 3. Bench verification

- [x] 3.1 Build and flash via the external ST-LINK V3, confirm
      `bno085_bringup()` returns `HAL_OK` and reports a plausible SHTP
      header (channel 0, non-zero length) over USART3
