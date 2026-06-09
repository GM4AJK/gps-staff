# Nucleo-F767ZI Sandbox

A bare CubeMX project targeting the STM32F767ZI Nucleo board, used as a
"play pen" for developing and bench-testing modules before porting them
into `rtk-base`/`rtk-rover` (pin assignments, peripherals, etc. will
differ slightly between boards).

## Programming / debugging

The on-board ST-LINK has been physically cut away from this board (freed
up to program other STM32 targets). An external **ST-LINK V3 programmer**
is wired directly to the Morpho headers instead:

| MCU Pin | Signal | Role | Header / Pin | Prog |
|---|---|---|---|---|
| 3V3  | PWR         | Power      | CN11 pin 12 | CN2 Pin 1 |
| GND  | GND         | Ground     | CN11 pin 20 | CN2 Pin 20 |
| PA13 | SWDIO (TMS) | SWD data | CN11 pin 13 | CN2 Pin 7 |
| PA14 | SWCLK (TCK) | SWD clock | CN11 pin 15 | CN2 Pin 9 |
| NRST | Reset | SWD reset | CN11 pin 14 | CN2 Pin 15 |
| PD8  | USART3_TX → ST-LINK RX | VCP UART, MCU TX -> probe RX | CN12 pin 10 | CN3 Pin 2 |
| PD9  | USART3_RX ← ST-LINK TX | VCP UART, probe TX -> MCU RX | CN11 pin 67 | CN3 Pin 1 |

PA13/PA14/NRST are the MCU's native SWD pins; PD8/PD9 are the Nucleo-144's
standard Virtual COM Port (USART3) pins, so the same connection also
carries the VCP UART through to the external probe.

## Power supply (bench PSU)

With the ST-LINK section cut away there is no USB power. Two external
options (UM1974 Table 7):

| Input | Pin | Voltage | Max current | JP3 setting |
|---|---|---|---|---|
| E5V | CN11 pin 6 | 4.75–5.25 V | 500 mA | Pins 1–2 bridged |
| V_IN | CN8 pin 15 | 7–12 V | 800 mA @ 7 V | Pins 5–6 bridged |

**Use V_IN (CN8 pin 15)** — JP3 pins 5–6 are already bridged on this board
(confirmed from physical inspection), and CN8 pin 15 is the pin that has
previously been used.

Required jumper state before applying power:
- **JP3**: pins 5–6 bridged (V_IN position — already set)
- **JP1**: remove jumper (OFF)

Set bench PSU to **7.0 V, 500 mA** current limit. The onboard LD1117
regulates this to 5 V, which then feeds the 3.3 V LDO. The green LED LD6
lights when the board is correctly powered.

Full reference: UM1974 STM32 Nucleo-144 boards user manual,
Section 7.4 — `docs/datasheets/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics-3.pdf`
