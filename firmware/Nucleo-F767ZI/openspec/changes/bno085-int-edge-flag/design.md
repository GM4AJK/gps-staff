## Context

`bno085_reset_and_wait()` (used by `bno085_bringup()` and
`bno085_read_advertisement()`) and `bno085_wait_int_low()` (used via
`bno085_wake_and_wait_int_low()` by `bno085_send_packet()` and
`bno085_read_response()`) all currently determine "`INT` is asserted"
by polling `HAL_GPIO_ReadPin(int_port, int_pin) == GPIO_PIN_RESET` in a
tight loop with a millisecond timeout.

Bench testing shows intermittent `HAL_TIMEOUT` failures from
`bno085_bringup()`/`bno085_read_advertisement()` on cold start,
requiring one or more MCU resets before the BNO085 comes up cleanly.
The suspected mechanism: the BNO085's first post-reset `INT` assertion
(carrying its automatic advertisement/initial-report burst) can be a
brief pulse relative to the host's poll loop overhead - if it occurs
before the low-wait loop starts polling (e.g. during the `RST` pulse,
or during the existing 50ms "deassert debounce" window in
`bno085_reset_and_wait()`), a pure level poll never observes the low
level and the loop runs out the full `BNO085_INT_TIMEOUT_MS`. Because
the BNO085 won't assert `INT` again on its own (subsequent assertions
require a `PS0`/`WAKE` pulse, per `bno085-get-feature-enum`), this
timeout is unrecoverable without a reset.

`app.c` already has `HAL_GPIO_EXTI_Callback()` wired to `INT`'s falling
edge (`GPIO_MODE_IT_FALLING`, configured in `MX_GPIO_Init()`), currently
setting an ad-hoc `volatile bool BNO085_INT_STATE`. EXTI edges are
latched in hardware and serviced by the NVIC as soon as interrupts are
enabled, independent of when application code starts polling - so a
flag set from this ISR can capture an assertion that a level poll would
miss.

The project already has a small atomic flag module (`flags.h`/`flags.c`)
using `stdatomic.h` (`atomic_fetch_or`/`atomic_fetch_and` on a
`volatile uint32_t`) with a `MAKE_H`/`MAKE_C` macro pair generating
`flag_set_<X>()` / `flag_get_<X>()` (test-and-clear) / `flag_peek_<X>()`
for each flag. Adding a `BNO085_INT` flag via these macros gives an
IRQ-safe set/test-and-clear for free, matching the existing idiom.

## Goals / Non-Goals

**Goals:**
- Eliminate the missed-first-assertion race so `bno085_bringup()` /
  `bno085_read_advertisement()` reliably succeed on cold start without
  requiring an MCU reset.
- Reuse the existing `flags.h`/`flags.c` atomic flag pattern rather than
  introducing a new synchronization primitive.
- Keep the change confined to the `INT`-wait helpers; no change to
  public function signatures, return codes, or the SPI transfer logic
  itself.

**Non-Goals:**
- No change to the `PS0`/`WAKE` pulse logic itself
  (`bno085_wake_and_wait_int_low()`'s decision of *when* to pulse WAKE
  is unchanged - only how it then waits for `INT`).
- No attempt to handle multiple queued/missed `INT` assertions (the
  flag is a single sticky bit, not a counter) - one latched edge is
  sufficient to unblock one wait.
- Not addressing `BNO085_INT_TIMEOUT_MS`'s value (currently 5000ms,
  bumped during earlier diagnosis) - left as-is; a future change can
  revisit it once this race is confirmed fixed on the bench.

## Decisions

- **New `BNO085_INT` flag via existing macros**: add
  `MAKE_H(BNO085_INT)` / `MAKE_C(BNO085_INT)` to `flags.h`/`flags.c`,
  giving `flag_set_BNO085_INT()`, `flag_get_BNO085_INT()` (test-and-clear),
  and `flag_peek_BNO085_INT()`. `HAL_GPIO_EXTI_Callback()` in `app.c`
  calls `flag_set_BNO085_INT()` on `INT`'s falling edge, replacing the
  ad-hoc `BNO085_INT_STATE` global (removed).
- **"Drain after the in-reset/in-progress low period, consume on wait"
  pattern**: call `flag_get_BNO085_INT()` once to discard any stale
  latched edge immediately before the wait loop begins, but *after* any
  period during which the device is known/expected to drive or read
  `INT` low for reasons unrelated to data-ready:
  - In `bno085_reset_and_wait()`, the BNO085 can read/drive `INT` low
    for some of the `RST` pulse and the in-reset/booting period
    afterwards (this is exactly what the existing 50ms
    "deassert-debounce" loop waits out at the level layer). The drain
    therefore happens *after* the deassert-debounce loop, immediately
    before calling `bno085_wait_int_low()` - not before releasing `RST`.
    Draining before `RST` release was tried first and made every
    `bno085_bringup()`/`bno085_read_advertisement()` call return
    instantly with `int_wait_ms == 0` and garbage data: the in-reset low
    edge latched the flag during the `RST` pulse, so
    `bno085_wait_int_low()` immediately reported "ready" while the chip
    was still booting.
  - In `bno085_wake_and_wait_int_low()`, there is no equivalent
    in-progress low period before the `PS0`/`WAKE` pulse, so the drain
    happens immediately before pulsing `PS0`/`WAKE` low, as originally
    designed.

  The subsequent wait loop's success condition is
  `flag_get_BNO085_INT() || HAL_GPIO_ReadPin(int_port, int_pin) ==
  GPIO_PIN_RESET` - i.e. succeed if either an edge was latched at any
  point since the drain, or `INT` currently reads low. This closes the
  race regardless of whether the (real, post-boot) assertion happens
  before, during, or after the wait loop starts polling.
  - *Alternative considered*: drain the flag at the *start* of the wait
    loop instead of before the triggering action (RST release / WAKE
    pulse). Rejected - if the assertion happens between the triggering
    action and the start of the wait loop (the exact race being fixed),
    draining at wait-loop-start would discard the very edge we need to
    observe.
  - *Alternative considered*: only use the flag (drop the level check).
    Rejected - if `INT` is already low *before* the drain point (e.g.
    `bno085_wake_and_wait_int_low()`'s "already low, no wake needed"
    fast path), no new falling edge will occur, so the flag alone would
    never become set; the level check is still required for that case.
- **`bno085_wait_int_low()` becomes the single low-level wait
  primitive**: `bno085_reset_and_wait()`'s existing inline "wait for
  `INT` low within `BNO085_INT_TIMEOUT_MS`" loop is replaced with a call
  to `bno085_wait_int_low()` (moved earlier in `bno085.c`, or
  forward-declared), after draining the flag and releasing `RST`. This
  removes duplicated polling logic and ensures both wait sites use the
  same flag-or-level condition. The existing 50ms "deassert debounce"
  loop in `bno085_reset_and_wait()` is unchanged (it is unrelated to the
  flag - it only rejects an immediate-low *level* reading while the
  chip is still in reset/boot).
- **No drain inside `bno085_wait_int_low()` itself**: draining happens
  in the caller (`bno085_reset_and_wait()` / `bno085_wake_and_wait_int_low()`)
  immediately before the action that can trigger an assertion, not
  inside `bno085_wait_int_low()` - otherwise a fast assertion that
  happens between the triggering action and the call to
  `bno085_wait_int_low()` would be drained away before the wait loop
  ever checks it.

## Risks / Trade-offs

- [If `INT`'s NVIC interrupt is not actually enabled/serviced before
  `bno085_reset_and_wait()` runs (e.g. interrupts globally disabled
  during early boot, or the EXTI line not yet configured at the point
  `app_init()` calls into the BNO085 driver), the flag would never be
  set and behavior falls back to the existing level-only check] →
  `MX_GPIO_Init()` (which configures `GPIO_MODE_IT_FALLING` and enables
  the NVIC line) runs before `app_init()`, and global interrupts are
  enabled by the time `app_init()` runs, so this should not occur in
  practice; bench testing will confirm.
- [A spurious falling edge unrelated to a real data-ready condition
  (e.g. electrical noise) could set the flag and cause a wait to
  succeed when `INT` isn't actually asserted, leading to a SPI read of
  garbage] → the subsequent SPI read still parses an SHTP header and
  validates lengths/report IDs as today; a bogus header would be caught
  by existing length/ID checks (e.g. `bno085_get_feature()`'s
  retry/discard loop). This risk already exists today via the level
  check (a glitch could read low transiently) and is not made worse.
- [The flag is a single sticky bit - if multiple edges occur before a
  wait consumes it, only "at least one edge happened" is recorded, not
  "how many"] → sufficient for this use case, where each wait is for
  "has `INT` asserted at least once since I armed it".
