## Context

The BNO085 is wired to SPI1 (PA5/PA6/PA7), with CS on PD14 (software NSS,
idle high), RST on PD15 (active low, held low at boot), and an active-low
`INT` (data-ready) on PF12 configured for falling-edge EXTI in CubeMX. P0/P1
are bridged high on the breakout for SPI mode. This change is a minimal
bring-up, modeled on the `ssd1309-display-bringup` capability: prove the
wiring and SPI configuration by resetting the chip and reading the SHTP
packet it sends afterwards, without implementing the full SHTP/sensor-report
protocol.

## Goals / Non-Goals

**Goals:**
- `bno085_init()`: populate a `bno085_t` handle with the SPI handle pointer
  and the CS/RST/INT GPIO ports/pins, performing no transactions.
- `bno085_bringup()`: drive the reset sequence, wait for the chip to signal
  data-ready via `INT`, and read the resulting SHTP packet (header +
  payload, up to a fixed buffer size) into the handle.
- A demo in `app_init()` that runs bring-up and reports success/failure plus
  the parsed SHTP header (length, channel, sequence number) over USART3.

**Non-Goals:**
- No SHTP channel/report parsing beyond the header fields needed to sanity
  check the response (length, channel, sequence number).
- No interrupt-driven (EXTI ISR) data path yet -- `bno085_bringup()` polls
  the `INT` GPIO level with a timeout. The full sensor driver can switch to
  an EXTI-flag-driven design later.
- No sensor configuration (feature reports, calibration, dynamic range,
  etc.) -- that's the subject of a future change once bring-up is verified
  on the bench.

## Decisions

- **Reset sequence**: drive `RST` low for `BNO085_RESET_PULSE_MS` (10 ms,
  comfortably above the chip's minimum reset pulse width), then drive it
  high and wait up to `BNO085_INT_TIMEOUT_MS` (1000 ms, a conservative
  upper bound) for `INT` to go low before giving up.
- **Polling INT**: for bring-up, `bno085_bringup()` polls
  `HAL_GPIO_ReadPin()` on the INT pin in a loop with `HAL_GetTick()`-based
  timeout, rather than relying on the EXTI interrupt configured in CubeMX.
  This keeps the bring-up self-contained and synchronous; the EXTI/NVIC
  config remains in place for the future interrupt-driven driver.
- **Single fixed-size SPI read**: once `INT` is low, assert `CS`, perform one
  `HAL_SPI_TransmitReceive()` of `BNO085_BRINGUP_BUF_SIZE` (32) bytes
  (transmitting all-zero dummy bytes), then deassert `CS`. The first 4 bytes
  are the SHTP header (16-bit length LSB-first with the continuation bit
  masked off, channel number, sequence number); remaining bytes are stored
  as-is. 32 bytes is enough to capture the header and the start of the
  post-reset advertisement payload for a sanity check, without needing to
  handle SHTP continuation reads.
- **Handle fields**: `bno085_t` stores the SPI handle pointer, CS/RST/INT
  GPIO ports/pins, the raw 32-byte receive buffer, and parsed
  `shtp_length`/`shtp_channel`/`shtp_sequence` fields from the last
  `bno085_bringup()` read.

## Risks / Trade-offs

- [Polling INT blocks `app_init()` for up to 1 second on failure] →
  acceptable for a one-shot bring-up check; the full driver will move to an
  interrupt/flag-based design.
- [32-byte buffer may truncate a longer advertisement packet] → acceptable
  for bring-up, which only needs to confirm the SHTP header is well-formed
  and SPI framing is correct; full advertisement parsing is a non-goal here.
- [Reset pulse / INT timeout constants are estimates pending bench
  measurement] → exposed as named constants in `bno085.h` so they're easy to
  tune after the first bench test.
