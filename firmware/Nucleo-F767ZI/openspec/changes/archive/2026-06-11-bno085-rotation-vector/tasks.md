## 1. Constants and struct fields

- [x] 1.1 Add `BNO085_CHANNEL_INPUT_REPORTS` (3), Set Feature report ID
      (`0xFD`), and Rotation Vector report ID (`0x05`) constants to
      `bno085.h`.
- [x] 1.2 Add `bno085_rotation_vector_t` struct (raw `i`/`j`/`k`/`real`/
      `accuracy` as `int16_t`, `i_f`/`j_f`/`k_f`/`real_f`/`accuracy_rad` as
      `float`, `sequence` and `status` as `uint8_t`) to `bno085.h`.
- [x] 1.3 Add a `rotation_vector` field of type `bno085_rotation_vector_t` to
      `bno085_t`.

## 2. Set Feature command

- [x] 2.1 Implement `bno085_set_feature()` in `bno085.c`: build the 17-byte
      Set Feature Command payload and send it via `bno085_send_packet()` on
      `BNO085_CHANNEL_CONTROL`.
- [x] 2.2 Document `bno085_set_feature()` in `bno085.h` following the
      existing Doxygen-style comment conventions.

## 3. Rotation Vector input report

- [x] 3.1 Implement `bno085_read_rotation_vector()` in `bno085.c`: loop
      `bno085_read_response()` up to `BNO085_GET_FEATURE_MAX_ATTEMPTS` times,
      matching on channel/length/report IDs as specified, and parse the
      matching packet into `rotation_vector`.
- [x] 3.2 Document `bno085_read_rotation_vector()` in `bno085.h`.

## 4. Debug printing

- [x] 4.1 Implement `bno085_print_rotation_vector()` in `bno085.c`,
      following the `bno085_print_advertisement()` pattern.
- [x] 4.2 Document `bno085_print_rotation_vector()` in `bno085.h`.

## 5. Wire-up and review

- [x] 5.1 In `Core/Src/app.c`, after bring-up, call `bno085_set_feature()` to
      enable the Rotation Vector report (0x05) at a sensible report
      interval, then periodically call `bno085_read_rotation_vector()` and
      `bno085_print_rotation_vector()`.
- [x] 5.2 Bench-test on hardware: verify the Set Feature command is
      accepted, rotation vector reports are received, and printed values
      change sensibly as the board is moved.
