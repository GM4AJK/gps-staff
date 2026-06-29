# System Architecture — GPS Survey Staff

## Overview

A DIY RTK GNSS survey staff built around the u-blox ZED-F9P-05B module on a custom carrier PCB.
Two identical PCBs — one configured as base station, one as rover — communicate via LoRa.
Centimetre-level relative accuracy is the target.

---

## System Diagram

![System overview block diagram](system-overview.png)

```
BASE STATION [identical PCB]          ROVER (staff/pole) [identical PCB]
+-------------------------+           +-------------------------+
|  ZED-F9P (base mode)    |           |  ZED-F9P (rover mode)   |
|  RTCM out on UART       |           |  receives RTCM, RTK fix |
+--------+----------------+           +--------+----------------+
         | UART                                | UART
+--------v----------------+  LoRa RTCM -->   +--------v----------------+
|   STM32F765VIT6         |  <-- LoRa CMD    |   STM32F765VIT6         |
|   RTCM reader, LoRa TX  |                  |   RTCM->F9P, SD log     |
|   survey-in control     |                  |   OLED: tilt/fix/status |
+--------+----------------+                  +--------+----------------+
         | UART (RTCM)                                | UART
+--------v----------------+                +----------v--------------+
|   ESP32-S3 Zero         |                |   ESP32-S3 Zero         |
|   WiFi → NTRIP caster   |                |   BLE 5.0 GATT server   |
|   BLE WiFi provisioning |                |   (position, status,    |
+--------+----------------+                |    command channel)     |
         v                                 +----------+--------------+
  NTRIP caster                                        | BLE
  (RTK2GO, Emlid, etc.)                   +----------v--------------+
                                          |   Handheld data         |
                                          |   collector             |
                                          |   4.3" ESP32-S3 (HH)   |
                                          |   7" ESP32-S3 (tablet)  |
                                          +-------------------------+
```

- Single PCB design; identical STM32 firmware on both units; mode (base/rover) selected by hardware jumper at boot
- Different ESP32-S3 Zero firmware per role — same module on both PCBs
- F9P handles all RTK computation onboard — STM32F7 is smart pipe + data logger

---

## Unit Roles

| Unit | Firmware | Role |
|------|----------|------|
| Base station | `rtk-base` + `esp32-base` | Surveys-in, enters fixed mode, TX RTCM over LoRa; streams RTCM to NTRIP caster over WiFi in gateway mode |
| Rover (staff) | `rtk-base` (rover mode) + `esp32-base` (rover role) | Receives RTCM from Base, passes to F9P, logs RTK-corrected positions to SD card |
| Handheld (4.3") | `esp32-handheld` | Survey UI: position readout, WiFi provisioning for Base, session management |
| Tablet (7") | `esp32-tablet` (TBD, arriving ~2026-07-01) | Full-resolution survey UI on 1024×600 display |

For detailed implemented capabilities, see:
[BASE.md](BASE.md) · [ROVER.md](ROVER.md) · [HANDHELD.md](HANDHELD.md)

---

## Key Technology Decisions

### GNSS: ZED-F9P-05B

L1/L2 dual-frequency RTK module. Handles all RTK computation onboard; STM32 is a data pipe.
RTCM constellation: GPS + GLONASS only — reduces 1Hz stream from ~700 bytes (4-GNSS) to
~255-325 bytes, fitting 2 SX1262 LoRa packets instead of 3-4.

### MCU: STM32F765VIT6

100-pin LQFP, 2MB flash, 512KB RAM, 216MHz Cortex-M7. Same F7 family as the Nucleo-F767ZI
dev board — firmware port to PCB is pin reassignment only. Single unified PCB for both
base and rover; mode selected by jumper.

### Radio: SX1262 (Waveshare Core1262-LF, 410-510MHz)

22dBm output, -148dBm sensitivity — massively over-specified for 100m working range.
SF7/BW500: ~25ms time-on-air, ~2.5% duty cycle at 1Hz — within ISM no-duty-cycle sub-band
(434.04-434.79MHz, 10mW). Full 22dBm available under GM4AJK amateur licence on 70cm band.

**Regulatory basis:** The project at SF7/BW500/+22dBm does not fit IR2030/1/10 (10mW limit),
IR2030/1/11 (1mW limit), or IR2030/1/12 (BW ≤ 25kHz required for 100% duty cycle). Operation
is under the GM4AJK amateur radio licence on the 70cm band. See `docs/datasheets/Ofcom-IR-2030.pdf`.

### BLE / WiFi Coprocessor: ESP32-S3 Zero

Fitted on both PCBs (identical build); firmware differs by role.
Base: WiFi NTRIP gateway + BLE WiFi provisioning server.
Rover: BLE 5.0 GATT server to handheld (position, status, command channel).

### Power: TPS63020 + BQ24075

TPS63020 buck-boost gives stable 3.3V across full LiPo discharge curve.
BQ24075 provides power-path management — USB-C 5V charging and operation simultaneously,
no glitch on hot-plug. EN1/EN2 firmware-controlled for USB current negotiation.

### IMU: LSM6DSRX + MMC5603NJ

LSM6DSRX (6-DOF) for tilt; MMC5603NJ for magnetic heading.
Combined purpose: pole lean correction — computes horizontal offset of ground contact point
when pole is not perfectly vertical.

Lean correction (simplified):
```
gravity = normalize(accel_xyz)
tilt_angle = arccos(gravity.z)
tilt_azimuth = heading + arctan2(gravity.y, gravity.x)
offset = pole_height × sin(tilt_angle)
```

At 2m pole height: 1° lean = 35mm uncorrected error; IMU correction reduces residual to ~3.5mm.

MMC5603NJ replaces the obsolete LIS3MDL (no longer purchasable). Connected via MMC560x-B
sub-board (J11) — avoids hand-soldering the 0.8mm WLP die.

### Data Logging: SDMMC 4-bit + FatFS

SDMMC1 on fixed AF pins (PC8-12/PD2). 4-bit bus chosen for USB MSC read path throughput;
SD write at 1Hz is trivially low for any interface. FatFS + STM32 HAL SDMMC + DMA.

### Configuration Storage: SD Card

Config file on SD card (pole height, LoRa band, calibration coefficients). No EEPROM.
VBAT wired directly to 3.3V — backup domain available while powered, lost on power-off;
no cross-power-cycle retention needed since config is on SD.

---

## Firmware Architecture

Three CubeIDE projects:

| Project | Target | Role |
|---------|--------|------|
| `rtk-base` | STM32F765VIT6 (custom PCB) | Real firmware — base and rover mode in one binary |
| `rtk-sandbox` | STM32F765VIT6 (custom PCB) | CubeMX code reference — never flashed |
| `Nucleo-F767ZI` | STM32F767ZI (Nucleo-144) | Hardware sandbox — bench-test peripherals before porting |

CubeMX owns: clock tree, mode-select GPIO, debug UART, SWO. Everything else is application-owned,
initialised at register level. No `MX_*_Init()` beyond those four. This cleanly solves the
single-firmware / dual-hardware problem: the app reads the mode jumper and brings up only the
peripherals the physical assembly has.

Full firmware module documentation: [`docs/arch/`](../../docs/arch/)

---

## RTK Concepts

- **Code-phase** (consumer GPS): ~1-5m — noise floor cannot be corrected below ~1m
- **Carrier-phase** (RTK): mm measurement noise — resolves integer ambiguity for cm positions
- **RTK base position**: affects *absolute* accuracy only, not *relative* accuracy between points
  - Unknown base: cm relative accuracy, offset absolute position
  - Known base (benchmark / NTRIP): cm absolute accuracy in OS datum
- **NTRIP**: UK correction networks available (OS Net) for absolute coordinate accuracy
