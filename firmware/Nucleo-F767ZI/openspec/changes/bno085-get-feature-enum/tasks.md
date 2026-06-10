## 1. Driver: SHTP packet send

- [x] 1.1 Add `tx_seq[6]`, `cmd_buf[BNO085_CMD_BUF_SIZE]` (32),
      `BNO085_CHANNEL_CONTROL` (= 2), `bno085_feature_t`, and the
      `feature` field to `bno085_t` in `bno085.h`.
- [x] 1.2 Implement `bno085_send_packet()` in `bno085.c`: build the
      4-byte SHTP header from `tx_seq[channel]`, send header + payload
      in one `HAL_SPI_TransmitReceive()` with `CS` asserted/released,
      and increment `tx_seq[channel]` on success.

## 2. Driver: Get Feature

- [x] 2.1 Implement `bno085_get_feature()` in `bno085.c`: send the
      Get Feature Request via `bno085_send_packet()`, wait for `INT`
      low (`BNO085_INT_TIMEOUT_MS`), read `cmd_buf`, and parse a
      matching `0xFC` response into `feature`.
- [x] 2.2 Declare `bno085_send_packet()` and `bno085_get_feature()` in
      `bno085.h` with doc comments describing parameters, buffers
      touched, and return codes.

## 3. Demo

- [x] 3.1 In `app_init()`, after the advertisement read, call
      `bno085_get_feature()` for Accelerometer (`0x01`), Gyroscope
      (`0x02`), Magnetic Field (`0x03`), Rotation Vector (`0x05`), and
      Game Rotation Vector (`0x08`), printing each result
      (`report_interval_us` and other `feature` fields, or the error
      status) over USART3.

## 4. Bench verification

- [x] 4.1 Bench-test on hardware and confirm Get Feature
      responses are received and parsed for at least one enabled
      sensor (non-zero `report_interval_us`).
