# bno085-periodic-dcd-save

## Purpose

Enables the BNO085's periodic Dynamic Calibration Data (DCD) save feature
via the SH-2 Configure Periodic DCD Save (0x09) command, so that
calibration data accumulated by the on-device fusion engine is
periodically persisted to non-volatile storage without host
intervention.

## Requirements

### Requirement: Send a Configure Periodic DCD Save command
The driver SHALL provide `bno085_set_periodic_dcd_save(bno085_t *p,
uint8_t enable)`, which sends an SH-2 Configure Periodic DCD Save (0x09)
command via `bno085_send_command()` with parameters `{enable ? 0x00 :
0x01, 0, 0, 0, 0, 0, 0, 0, 0}`. Per the SH-2 Reference Manual section
6.4.7, this command has no response, so
`bno085_read_command_response()` SHALL NOT be called.

#### Scenario: Enable periodic DCD save
- **WHEN** `bno085_set_periodic_dcd_save(p, 1)` is called
- **THEN** a Command Request (0xF2) with command `0x09` and P0 = `0x00`
  is sent on `BNO085_CHANNEL_CONTROL`
- **AND** the function returns the result of `bno085_send_command()`
  without waiting for a response

#### Scenario: Disable periodic DCD save
- **WHEN** `bno085_set_periodic_dcd_save(p, 0)` is called
- **THEN** a Command Request (0xF2) with command `0x09` and P0 = `0x01`
  is sent on `BNO085_CHANNEL_CONTROL`
- **AND** the function returns the result of `bno085_send_command()`
  without waiting for a response

### Requirement: Enable periodic DCD save at startup
`app_init()` SHALL call `bno085_set_periodic_dcd_save(&bno, 1)` after the
existing ME calibration enable call, and print the result over USART3.

#### Scenario: Periodic DCD save enabled at startup
- **WHEN** `app_init()` runs
- **THEN** `bno085_set_periodic_dcd_save(&bno, 1)` is called
- **AND** `bno085_set_periodic_dcd_save OK` (or a failure status) is
  printed over USART3
