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
- [x] 3.2 In `bno085_reset_and_wait()`, call `flag_get_BNO085_INT()` to
      discard any stale edge immediately before releasing `RST`, then
      replace the existing inline "wait for INT low" loop with a call
      to `bno085_wait_int_low()`. Leave the deassert-debounce loop
      unchanged.
- [x] 3.3 In `bno085_wake_and_wait_int_low()`, call
      `flag_get_BNO085_INT()` to discard any stale edge immediately
      before pulsing `PS0`/`WAKE` low (only on the path where `INT` is
      not already low).

## 4. Bench verification

- [ ] 4.1 Bench-test repeated cold-start/reset cycles and confirm
      `bno085_bringup()`/`bno085_read_advertisement()` no longer
      intermittently return `HAL_TIMEOUT`.
