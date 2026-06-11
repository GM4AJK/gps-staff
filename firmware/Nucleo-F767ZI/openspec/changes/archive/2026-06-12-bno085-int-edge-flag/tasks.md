## 1. Flag module

- [x] 1.1 Add a `BNO085_INT` flag to `flags.h`/`flags.c` via the
      existing `MAKE_H`/`MAKE_C` macros, giving `flag_set_BNO085_INT()`,
      `flag_get_BNO085_INT()`, and `flag_peek_BNO085_INT()`.

## 2. EXTI callback

- [x] 2.1 In `app.c`, replace `BNO085_INT_STATE = true` in
      `HAL_GPIO_EXTI_Callback()` with `flag_set_BNO085_INT()`, and
      remove the `BNO085_INT_STATE` global.

## 3. Driver: flag-or-level INT wait

- [x] 3.1 Move/forward-declare `bno085_wait_int_low()` so it can be
      called from `bno085_reset_and_wait()`; change its loop condition
      to succeed when `flag_get_BNO085_INT()` returns true OR `INT`
      reads low (recording `int_wait_ms` as today).
- [x] 3.2 In `bno085_reset_and_wait()`, replace the existing inline
      "wait for INT low" loop with a call to `bno085_wait_int_low()`,
      and call `flag_get_BNO085_INT()` to discard any stale edge
      immediately after the deassert-debounce loop (not before
      releasing `RST` - the device can read/drive `INT` low during the
      `RST` pulse and in-reset/booting period, which the
      deassert-debounce loop waits out at the level layer).
- [x] 3.3 In `bno085_wake_and_wait_int_low()`, call
      `flag_get_BNO085_INT()` to discard any stale edge immediately
      before pulsing `PS0`/`WAKE` low (only on the path where `INT` is
      not already low).

## 4. Startup timing fix

- [x] 4.1 Add `BNO085_STARTUP_T1_MS` (90, datasheet 6.5.3 t1) to
      `bno085.h`. In `bno085_reset_and_wait()`, pulse `RST` low for
      `BNO085_RESET_PULSE_MS` then high, then wait
      `BNO085_STARTUP_T1_MS` before sampling `INT` at all (it is
      undefined during this period) - repeat this pulse+wait twice
      (a second `RST` pulse is required to reliably recover from a
      power cycle), before the existing deassert-debounce loop runs.

## 5. Bench verification

- [x] 5.1 Bench-test repeated cold-start/reset cycles (flash reset,
      Nucleo reset-button, and full power cycle) and confirm
      `bno085_bringup()`/`bno085_read_advertisement()` consistently
      return real (non-`0xFF`) data with no `HAL_TIMEOUT`.
