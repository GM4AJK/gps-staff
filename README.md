# GPS Survey Staff

A DIY RTK GNSS survey staff -- a handheld device for centimetre-level accurate position surveying.

## What is this?

The system consists of two identical handheld units: one set up as a fixed base station, the other carried by the surveyor as a rover. The two communicate wirelessly so the rover can compute corrected, centimetre-accurate positions in real time (RTK -- Real-Time Kinematic positioning).

It's designed to be self-contained and portable -- battery powered, with an onboard display, simple button/encoder controls, and onboard logging of survey data for later processing.

## Project status

Active development. PCB v1.0 ordered 2026-06-25. Firmware bring-up ongoing on Nucleo dev boards.

## Documentation

| Location | Contents |
|----------|----------|
| [`sdd/`](sdd/) | System design: architecture, unit specs, BLE protocols, UI screen specs |
| [`docs/arch/`](docs/arch/) | Firmware architecture: RTCM3, OTA protocol, SX1262 driver, SD card |
| [`docs/datasheets/`](docs/datasheets/) | Component datasheets and reference documents |
| [`hardware/rtk/`](hardware/rtk/) | PCB electronics design: KiCad files, BOM, design notes |
