# Rover Unit — Implemented Capabilities

Covers both `firmware/esp32-base` (rover role) and `firmware/rtk-rover` (STM32 rover role).

> BLE services: see [GATT.md](GATT.md) for UUIDs, wire formats, and peripheral/central roles.

---

## ESP32-S3 Zero (firmware/esp32-base — rover role)

| Capability | Status | Notes |
|------------|--------|-------|
| Role selection at boot | Done | GP1 GND = rover; same binary as base |
| BLE role | Pending | BLE central to Handheld (status/position stream) — future spec; RTCM bridge over BLE removed, RTCM flows over GFSK |

## STM32 (firmware/rtk-rover)

| Capability | Status | Notes |
|------------|--------|-------|
| FreeRTOS scaffold | Done | Mirrors rtk-base structure |
| SX1262 GFSK RX | Done | Interrupt-driven DIO1 |
| RTCM3 receive + framing | Done | |
| SSD1309 OLED display | Done | Status display, I2C |

---

## Pending / In Spec

| Feature | Spec |
|---------|------|
| BLE link to Handheld (position/status stream) | Future spec |
| BLE link to Handheld (command/config channel) | Future spec |
| ZED-F9P UART driver + RTCM feed | Awaiting F9P delivery |
| MMC5603NJ magnetometer driver | Needed (LIS3MDL discontinued) |
| Tilt fusion / soft-iron calibration | Deferred |
