# Rover Unit — Implemented Capabilities

Covers both `firmware/esp32-base` (rover role) and `firmware/rtk-rover` (STM32 rover role).

---

## ESP32-S3 Zero (firmware/esp32-base — rover role)

| Capability | Status | Notes |
|------------|--------|-------|
| Role selection at boot | Done | GP1 GND = rover; same binary as base |
| BLE serial bridge (rover side) | Done PR #187 | Central role, scans for "GPS-Base", subscribes to notifications, forwards to UART |
| UART0 bridge to STM32 | Done PR #187 | GPIO43/44, pipes data between BLE and STM32 UART |

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
| WiFi provisioning relay (Handheld → Rover → Base) | [00001-wifi-provisioning.md](00001-wifi-provisioning.md) |
| ZED-F9P UART driver + RTCM feed | Awaiting F9P delivery |
| MMC5603NJ magnetometer driver | Needed (LIS3MDL discontinued) |
| Tilt fusion / soft-iron calibration | Deferred |
