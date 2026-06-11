## Why

Bench testing of the BNO085 driver shows intermittent startup failures
(`bno085_bringup()`/`bno085_read_advertisement()` returning
`HAL_TIMEOUT`) that require resetting the MCU one or more times before
the device comes up cleanly. The driver currently detects "`INT` is
asserted" purely by polling the GPIO level, which can miss a brief
falling edge that occurs before or during the polling loop's setup
(e.g. while `RST` is still being released, or during the post-reset
"deassert debounce" window). Once that first post-reset assertion is
missed, the BNO085 won't assert `INT` again on its own, and the
`INT`-wait loop runs out the full `BNO085_INT_TIMEOUT_MS` before giving
up. An EXTI falling-edge interrupt on `INT` (already wired up in
`app.c`'s `HAL_GPIO_EXTI_Callback()`) latches the edge in hardware
regardless of polling timing, so consuming that via a flag closes this
race.

## What Changes

- Add a `BNO085_INT` flag to the existing `flags.h`/`flags.c` module
  (using its existing `MAKE_H`/`MAKE_C` atomic set/get/peek macro
  pattern), set from `HAL_GPIO_EXTI_Callback()` on the `INT` pin's
  falling edge.
- Remove the ad-hoc `BNO085_INT_STATE` global from `app.c` in favour of
  the new flag.
- Update the BNO085 driver's `INT`-low wait points
  (`bno085_reset_and_wait()`, `bno085_wait_int_low()` /
  `bno085_wake_and_wait_int_low()`) to drain the flag immediately before
  the point where `INT` could first legitimately assert, then treat
  `INT` as "ready" once either the flag is set (an edge was latched) or
  the GPIO level reads low - whichever is observed first.

## Capabilities

### New Capabilities
(none)

### Modified Capabilities
- `bno085-spi-bringup`: the "Wait for data-ready and read SHTP packet"
  requirement's `INT`-wait condition changes from a pure GPIO-level poll
  to "flag set (EXTI falling edge latched) or GPIO level low", to avoid
  missing a brief pre-poll assertion.

## Impact

- `Core/Inc/flags.h`, `Core/Src/flags.c`: add `BNO085_INT` flag
  (set/get/peek functions via the existing macros).
- `Core/Src/app.c`: `HAL_GPIO_EXTI_Callback()` sets the new flag instead
  of `BNO085_INT_STATE`; remove the `BNO085_INT_STATE` global.
- `Core/Inc/bno085.h`, `Core/Src/bno085.c`: update `bno085_reset_and_wait()`
  and the `INT`-wait helper(s) used by `bno085_bringup()`,
  `bno085_read_advertisement()`, `bno085_send_packet()`, and
  `bno085_read_response()` to use the flag-or-level check.
- No change to public function signatures or return codes.
