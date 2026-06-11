## MODIFIED Requirements

### Requirement: Wake the device and wait for INT before a transaction
The driver SHALL provide a helper (`bno085_wake_and_wait_int_low()`)
that is called before any SPI transaction in `bno085_send_packet()` and
`bno085_read_response()`. If `INT` is already low, it returns `HAL_OK`
immediately. Otherwise it SHALL discard any previously-latched
`BNO085_INT` edge flag (`flag_get_BNO085_INT()`), pulse `PS0`/`WAKE` low
(asking the device to assert `INT`), wait for `INT` to become active
within `BNO085_INT_TIMEOUT_MS` - where `INT` is considered active as
soon as either the `BNO085_INT` edge flag is set (a falling edge was
latched by `HAL_GPIO_EXTI_Callback()` at any point since it was
discarded) or the `INT` GPIO pin currently reads low, whichever is
observed first - then return `PS0`/`WAKE` high before returning the
wait result. `bno085_init()` SHALL accept and store a GPIO port/pin for
`PS0`/`WAKE`, and `bno085_reset_and_wait()` SHALL drive it high at the
start of reset (it must be high from before reset until after the first
assertion of `INT` to select the SPI interface).

#### Scenario: INT already low requires no wake pulse
- **WHEN** `bno085_wake_and_wait_int_low()` is called and `INT` is
  already low
- **THEN** it returns `HAL_OK` without changing `PS0`/`WAKE`
- **AND** the `BNO085_INT` edge flag is not discarded

#### Scenario: INT high triggers a wake pulse
- **WHEN** `bno085_wake_and_wait_int_low()` is called and `INT` is high
- **THEN** the `BNO085_INT` edge flag is discarded, `PS0`/`WAKE` is
  driven low, then `INT` is polled for it to become active (edge latched
  or level low) for up to `BNO085_INT_TIMEOUT_MS`
- **AND** `PS0`/`WAKE` is returned high before the function returns,
  regardless of whether `INT` became active in time
- **AND** the function returns `HAL_OK` if `INT` became active, or
  `HAL_TIMEOUT` otherwise

#### Scenario: A falling edge latched before the level-poll loop starts is not missed
- **WHEN** `bno085_wake_and_wait_int_low()` is called, `INT` is
  initially high, the `BNO085_INT` edge flag is discarded and
  `PS0`/`WAKE` is pulsed low, and `INT` briefly pulses low and back high
  (latching the flag via `HAL_GPIO_EXTI_Callback()`) before the
  level-poll loop begins
- **THEN** `bno085_wake_and_wait_int_low()` returns `HAL_OK` without
  waiting for `BNO085_INT_TIMEOUT_MS`
