# rtk-sandbox

A CubeMX/CubeIDE reference project targeting the **STM32F765VIT6** — the exact MCU used on
the custom GPS staff PCB.

## Purpose

This project is **never flashed to a device**. Its sole purpose is to serve as a CubeMX
code-generation reference for the real target chip. When writing register-level peripheral
drivers for `rtk-base`, use this project to ask "what would CubeMX generate for this
peripheral on this exact MCU?" and read the output as a reference.

Useful for:
- Deriving register init sequences for peripherals not yet in `rtk-base`
- Checking pin alternate-function assignments on the F765VIT6 specifically
- Exploring clock configuration options via the CubeMX clock tree GUI
- Any other "what does the HAL do here?" questions before writing bare-metal code

## Relationship to other firmware projects

| Project | Target | Role |
|---|---|---|
| `rtk-base` | STM32F765VIT6 (custom PCB) | **Real firmware** — the only project ever flashed |
| `rtk-sandbox` | STM32F765VIT6 (custom PCB) | **CubeMX code reference** — this project |
| `Nucleo-F767ZI` | STM32F767ZI (Nucleo-144 board) | **Physical hardware sandbox** — bench bring-up of peripheral modules before porting to PCB |
