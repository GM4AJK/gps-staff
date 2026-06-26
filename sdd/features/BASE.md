# Base Unit — Implemented Capabilities

Covers both `firmware/esp32-base` (base role) and `firmware/Nucleo-F767ZI-FreeRTOS` / `firmware/rtk-base` (STM32 base role).

---

## ESP32-S3 Zero (firmware/esp32-base)

| Capability | Status | Notes |
|------------|--------|-------|
| Role selection at boot | Done | GP1 open = base, GP1 GND = rover; 500ms timeout then direct GPIO read |
| BLE serial bridge (base side) | Done PR #187 | Advertises as "GPS-Base", GATT peripheral, notifies rover of UART data |
| UART0 bridge to STM32 | Done PR #187 | GPIO43/44, pipes data between BLE and STM32 UART |

## STM32 (firmware/Nucleo-F767ZI-FreeRTOS / rtk-base)

| Capability | Status | Notes |
|------------|--------|-------|
| FreeRTOS scaffold | Done | app_init/app_loop/app_1ms hooks, flags.c IRQ bridge |
| SX1262 GFSK TX | Done | Interrupt-driven DIO1, active bench mode |
| RTCM3 framing + OTA | Done | RTCM3_BUF_COUNT=4; bump to 8 before adding display driver |
| FatFS SD card | Done | F767ZI bench-proven; FF_FS_NORTC set — needs GPS UTC when F9P arrives |
| LSM6DSOX IMU driver | Done PR #124 | I2C1 on F446RE bench |

---

## Pending / In Spec

| Feature | Spec |
|---------|------|
| WiFi provisioning (scan + BLE advertise) | [00001-wifi-provisioning.md](00001-wifi-provisioning.md) Phase 1 |
| WiFi connect + NVS credential storage | [00001-wifi-provisioning.md](00001-wifi-provisioning.md) Phase 3 |
| NTRIP RTCM streaming over WiFi | Future spec |
| PE2/PE3 mode-select GPIO (PCB) | Awaiting PCB delivery |
| ZED-F9P UART driver | Awaiting F9P delivery |
