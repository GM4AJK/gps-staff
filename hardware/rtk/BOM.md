# Schematic BOM — rtk PCB

Generated from KiCad schematics. One row per reference designator.
**Purpose:** footprint audit before PCB layout — verify every footprint against the actual part's datasheet land pattern.

Key:
- **Footprint** — KiCad library:footprint as currently assigned in the schematic
- **⚠** — suspected issue, needs checking before layout

---

## Capacitors

| Ref | Value | Footprint | Sheet | Notes |
|-----|-------|-----------|-------|-------|
| C1 | 1u | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | power | Tantallum (TAJR105M016RNJ) — verify 0805 land pattern matches AVX/Kyocera footprint |
| C2 | 4u7 | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | power | Tantalum (TAJP475K016RNJ) — same as C1 |
| C3 | 4u7 | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | power | Tantalum (TAJP475K016RNJ) — same as C1 |
| C4 | 100n | `C_0603_1608Metric_Pad1.08x0.95mm_HandSolder` | power | TPS63020 CvinA decoupling |
| C5 | 22u | `C_0603_1608Metric_Pad1.08x0.95mm_HandSolder` | power | TPS63020 Cin — part is C1608X5R1A226M080AC (0603/1608 metric) ✓ |
| C6 | 47u | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | power | TPS63020 Cout — part is MSASJ21GBB5476MTNA01 (0805/2012 metric) ✓ |
| C7 | 10n | `C_0603_1608Metric_Pad1.08x0.95mm_HandSolder` | zed-f9p-05b | VCC_RF decoupling |
| C8 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C9 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C10 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C11 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C12 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C13 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C14 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C15 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C16 | 10u | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | **T529P106M010AAE200** — Kemet KO-CAP polymer tant, 10µF 10V 0805, 200mΩ. Polarised symbol (C_Polarized_Small). ✓ |
| C17 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C18 | 10u | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | **T529P106M010AAE200** — same as C16 ✓ |
| C19 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C20 | 10u | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | **T529P106M010AAE200** — same as C16 ✓ |
| C21 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | VDD decoupling |
| C22 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 decoupling |
| C23 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 decoupling |
| C24 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 decoupling |
| C25 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 decoupling |
| C26 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | Decoupling |
| C27 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 decoupling |
| C28 | 470n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 VDD1.8 bulk |
| C29 | 470n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 VDD1.8 bulk |
| C30 | 100n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | usb2hs | USB3300 decoupling |
| C31 | 10n | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | power | TPS63020 soft-start / inrush (C_SS) |
| C32 | 1u | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | STM32 VCAP_1 (internal LDO) |
| C33 | 1u | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | stm32f765vit | STM32 VCAP_2 (internal LDO) |
| C34 | 100n | `C_0603_1608Metric_Pad1.08x0.95mm_HandSolder` | zed-f9p-05b | V_BCKP decoupling |

---

## Resistors

| Ref | Value | Footprint | Sheet | Notes |
|-----|-------|-----------|-------|-------|
| R1 | 1k13 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | BQ24075 ISET |
| R2 | 1K5 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | BQ24075 nPGOOD LED pull-up |
| R3 | 1K5 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | BQ24075 nCHG LED pull-up |
| R4 | 1k18 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | BQ24075 — check function |
| R5 | 46k4 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | BQ24075 TMR |
| R6 | 1k | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | LED current limit |
| R7 | 768k | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | TPS63020 Rfbt (FB top) |
| R8 | 137k | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | TPS63020 Rfbb (FB bottom) |
| R9 | 12k | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | usb2hs | USB3300 RBIAS |
| R10 | 10K | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | usb2hs | USB3300 VBUS protect |
| R11 | 5k1 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | USB-C CC1 pull-down |
| R12 | 5k1 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | USB-C CC2 pull-down |
| R13 | NF or 82k5 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | TPS63020 TS pin (NF or 82k5 to set threshold) |
| R14 | NF or 1M | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | power | TPS63020 bleed resistor (NF or 1M) |
| R15 | 1k | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | stm32f765vit | BOOT0 pull-down |
| R16 | 4k7 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | stm32f765vit | I2C4 SDA pull-up |
| R17 | 4k7 | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | stm32f765vit | I2C4 SCL pull-up |
| R18 | 10k | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | zed-f9p-05b | ZED-F9P SAFEBOOT_N pull-up |
| R19 | 10k | `R_0805_2012Metric_Pad1.20x1.40mm_HandSolder` | zed-f9p-05b | ZED-F9P RESET_N pull-up |

---

## Inductors / Ferrite Beads

| Ref | Value | Footprint | Sheet | Notes |
|-----|-------|-----------|-------|-------|
| L1 | 1uH | `IND_DFE2016_2X1P6_MUR` | power | TPS63020 switching inductor — verify custom footprint matches DFE201610E-1R0M |
| FB1 | FerriteBead_Small | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | zed-f9p-05b | ZED-F9P VCC_RF ferrite — 0805 cap footprint used for ferrite bead, verify land pattern |
| FB2 | FerriteBead_Small | `C_0805_2012Metric_Pad1.18x1.45mm_HandSolder` | zed-f9p-05b | ZED-F9P VCC ferrite — same as FB1 |

---

## Diodes / ESD / LED

| Ref | Value | Footprint | Sheet | Notes |
|-----|-------|-----------|-------|-------|
| D1 | 1N4148W | `D_SOD-123` | power | TPS63020 inrush diode |
| D2 | USBLC6-2SC6 | `SOT-23-6` | usb2hs | USB D+/D- dual-line ESD TVS — replaced PRTR5V0U2X (bulk only). D3 removed (one chip covers both lines) ✓ |
| LD1 | LED | `LED_0805_2012Metric_Pad1.15x1.40mm_HandSolder` | power | Status LED |
| LD2 | LED | `LED_0805_2012Metric_Pad1.15x1.40mm_HandSolder` | power | Status LED |

---

## ICs / Modules

| Ref | Value | Footprint | Sheet | Notes |
|-----|-------|-----------|-------|-------|
| U1 | BQ24075RGT | `VQFN-16-1EP_3x3mm_P0.5mm_EP1.6x1.6mm` | power | Battery charger / power path |
| U2 | TPS63020DSJR | `DSJ14_2P85X1P58` | power | Buck-boost — verify custom footprint matches VSON-14 land pattern in TPS63020 datasheet |
| U3 | STM32F765VITx | `LQFP-100_14x14mm_P0.5mm` | stm32f765vit | Main MCU |
| U4 | ZED-F9H-01B | `MOD54_ZED-F9H_UBL` | zed-f9p-05b | ⚠ **Part number: schematic says ZED-F9H-01B, project spec says ZED-F9P-05B** — confirm correct module |
| U5 | ESP32-S3-ZERO | `MODULE_ESP32-S3-ZERO` | stm32f765vit | BLE bridge module |
| U6 | CORE1262-868M | `XCVR_CORE1262-868M` | stm32f765vit | LoRa module — note: project uses LF (410-510 MHz) variant, confirm symbol/footprint match |
| U7 | LSM6DSOX | `PQFN50P250X300X86-14N` | stm32f765vit | IMU (accel + gyro) |
| U8 | USB3300-EZK | `QFN-32-1EP_5x5mm_P0.5mm_EP3.45x3.45mm` | usb2hs | ULPI USB PHY |

---

## Connectors

| Ref | Value | Footprint | Sheet | Notes |
|-----|-------|-----------|-------|-------|
| J1 | USB4105-GF-A | `USB4105_GCT` | power | USB-C receptacle (power in) |
| J3 | B2B-PH-SM4-TBLFSN | `CONN_B2B-PH-SM4-TBLFSN_JST` | power | Battery connector (JST PH 2-pin) |
| J4 | Conn_01x02_Pin | `PinHeader_1x02_P2.54mm_Vertical` | power | On/off switch header ✓ |
| J5 | Conn_Coaxial | `SMA_BAT_Wireless_BWSMA-KWE-Z001` | zed-f9p-05b | GNSS SMA |
| J6 | MEM2052-00-195-00-A_REVA | `GCT_MEM2052-00-195-00-A_REVA` | stm32f765vit | microSD card socket |
| J7 | Conn_01x04_Pin | `PinHeader_1x04_P2.54mm_Vertical` | stm32f765vit | Debug header |
| J8 | Conn_01x01_Pin | `PinHeader_1x01_P2.54mm_Vertical` | stm32f765vit | BOOT0 test point / header |
| J9 | Conn_01x08_Pin | `PinHeader_1x08_P2.54mm_Vertical` | stm32f765vit | Expansion / debug |
| J10 | Conn_01x02_Pin | `PinHeader_1x02_P2.54mm_Vertical` | stm32f765vit | Header |
| J11 | Conn_01x06_Pin | `PinHeader_1x06_P2.54mm_Vertical` | usb2hs | MMC5603NJ sub-board connector |

---

## Crystals / TCXOs

| Ref | Value | Footprint | Sheet | Notes |
|-----|-------|-----------|-------|-------|
| Y1 | ASTX-H11-16.000MHZ-T | `XTAL_ASTX-H11-16.000MHZ-T` | stm32f765vit | STM32 HSE TCXO (16 MHz) |
| Y2 | ASTX-H11-24.000MHZ-T | `XTAL_ASTX-H11-16.000MHZ-T` | usb2hs | USB3300 TCXO (24 MHz) — footprint is the 16 MHz variant name but same physical package ✓ |

---

## Items not in schematic (expected from project spec)

| Part | Notes |
|------|-------|
| AT24C256 (EEPROM) | Config store — not yet placed in schematic |
| MMC5603NJ | On MMC560x-B sub-board, connects via J11 — not placed as symbol |

---

## Issues to resolve before layout

| # | Ref | Issue |
|---|-----|-------|
| 1 | D2 | ~~PRTR5V0U2X SOT-143~~ **Fixed** — USBLC6-2SC6 SOT-23-6, D3 removed ✓ |
| 2 | C16, C18, C20 | ~~Through-hole electrolytic footprint~~ **Fixed** — T529P106M010AAE200 polymer tant, 0805 HandSolder ✓ |
| 3 | U4 | ~~Symbol says ZED-F9H-01B~~ **Confirmed OK** — same symbol and footprint as ZED-F9P-05B ✓ |
| 4 | J4 | ~~No footprint~~ **Fixed** — on/off switch, PinHeader_1x02_P2.54mm_Vertical ✓ |
| 5 | R2, R3 | ~~Value query~~ **Confirmed OK** — BQ24075 nPGOOD/nCHG open-drain LED pull-ups, 1K5 correct ✓ |
