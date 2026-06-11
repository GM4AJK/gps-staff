## ADDED Requirements

### Requirement: Enable a sensor report with Set Feature
The driver SHALL provide `bno085_set_feature()`, taking `report_id`,
`feature_flags`, `change_sensitivity`, `report_interval_us`,
`batch_interval_us`, and `sensor_specific_config`. It SHALL build a 17-byte
SH-2 Set Feature Command payload (`{0xFD, report_id, feature_flags,
change_sensitivity (2 bytes LE), report_interval_us (4 bytes LE),
batch_interval_us (4 bytes LE), sensor_specific_config (4 bytes LE)}`) and
send it on `BNO085_CHANNEL_CONTROL` via `bno085_send_packet()`. Unlike
`bno085_get_feature()`, no response is awaited - the SH-2 Get Feature
Response is only sent unsolicited on a later rate change, not synchronously
in reply to a Set Feature Command.

#### Scenario: Successful Set Feature send
- **WHEN** `bno085_set_feature()` is called with a `report_id` and feature
  parameters, and `bno085_send_packet()` succeeds
- **THEN** `bno085_set_feature()` returns `HAL_OK`
- **AND** the 17-byte payload sent on `BNO085_CHANNEL_CONTROL` is
  `0xFD` followed by `report_id`, `feature_flags`, and the four
  multi-byte fields encoded little-endian in the order
  `change_sensitivity`, `report_interval_us`, `batch_interval_us`,
  `sensor_specific_config`

#### Scenario: INT timeout during send is propagated
- **WHEN** `bno085_send_packet()` returns `HAL_TIMEOUT` (the wake/INT
  wait did not complete in time)
- **THEN** `bno085_set_feature()` returns `HAL_TIMEOUT` without retrying

#### Scenario: SPI failure during send is propagated
- **WHEN** the `HAL_SPI_TransmitReceive()` call inside
  `bno085_send_packet()` fails (error or timeout)
- **THEN** `bno085_set_feature()` returns the corresponding non-`HAL_OK`
  `HAL_StatusTypeDef`
