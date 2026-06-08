# Nucleo-F767ZI Sandbox

A bare CubeMX project targeting the STM32F767ZI Nucleo board, used as a
"play pen" for developing and bench-testing modules before porting them
into `rtk-base`/`rtk-rover` (pin assignments, peripherals, etc. will
differ slightly between boards).

## Programming / debugging

The on-board ST-LINK has been physically cut away from this board (freed
up to program other STM32 targets). An external **ST-LINK V3 programmer**
is wired directly to the **CN12** Morpho header instead:

| MCU Pin | Signal | Role |
|---|---|---|
| PA13 | SWDIO (TMS) | SWD data |
| PA14 | SWCLK (TCK) | SWD clock |
| PD8  | USART3_TX → ST-LINK RX | VCP UART, MCU TX -> probe RX |
| PD9  | USART3_RX ← ST-LINK TX | VCP UART, probe TX -> MCU RX |

(plus GND / VDD_TARGET / NRST as the standard SWD set requires).

PA13/PA14 are the MCU's native SWD pins; PD8/PD9 are the Nucleo-144's
standard Virtual COM Port (USART3) pins, so the same four-wire connection
also carries the VCP UART through to the external probe.
