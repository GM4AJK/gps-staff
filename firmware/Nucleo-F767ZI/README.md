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
