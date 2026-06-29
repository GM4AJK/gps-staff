# RTK PCB — Electronics Design

## PCB v1.0

- **Ordered:** 2026-06-25, JLCPCB. Tagged `PCB-v1.0` in git.
- **DigiKey BOM** ordered same date.
- **KiCad files:** `hardware/rtk/rtk/`
- **Component BOM:** [BOM.md](BOM.md)

### Board overview

| Property | Value |
|----------|-------|
| Layers | 4 (JLC04161H-7628 stackup) |
| Layer assignment | L1 top signal + components; L2 solid GND; L3 3.3V power plane; L4 bottom signal |
| RF traces | 50Ω microstrip on L1 (L2 GND reference); ~0.7mm trace width; JLCPCB controlled impedance |
| Design | Unified base + rover — single PCB, role selected by jumper at boot |
| Role-specific DNP | USB-C #1 (base: OTG FS) vs USB-C #2 + ULPI PHY (rover: OTG HS) |

### Unified PCB approach

Both units built from the same PCB. Mode selection:
- `PA0` (BASE_ROVER_MODE_SELECT): internal pull-up, link open = base, link to GND = rover
- Same STM32 firmware binary on both; mode gates peripheral init and code paths at runtime
- ESP32-S3 Zero firmware differs per role (base: WiFi NTRIP + BLE prov; rover: BLE GATT server)

---

## Schematic Connection Checklist

Consolidated list of every system wired on the schematic. Reference against the BOM and KiCad
to verify nothing is missed.

1. **ZED-F9P-05B** — UART1 (pins 42/43) → STM32 UART5 (nav + UBX config); UART2 (pins 26/27)
   → STM32 UART7 (PA8 RX ← F9P TXD2, PA15 TX → F9P RXD2) for RTCM input from LoRa;
   D_SEL (pin 47) → PE14 high (UART/I2C mode); RF_IN → GNSS SMA via 50Ω trace;
   1PPS → STM32 GPIO (log timestamping); power + decoupling.

2. **GNSS antenna (ANN-MB-00)** — SMA female through-hole; bias-T 3.3V LNA feed on RF line.

3. **LoRa (Core1262-LF / SX1262)** — dedicated SPI bus; BUSY checked before every transaction;
   DIO1/IRQ → STM32 GPIO; NRESET; separate SMA (433/434MHz).

4. **STM32F765VIT6** — UART (F9P + spare); 2× dedicated SPI (LoRa, IMU); SDMMC1 4-bit
   (PC8/D0, PC9/D1, PC10/D2, PC11/D3, PC12/CLK, PD2/CMD — fixed AF pins);
   I2C4 (PD12 SCL, PD13 SDA — display + IMU); USB OTG FS (PA11/PA12, base) or
   OTG HS + ULPI (rover); ADC (battery divider PA1); SWD.

5. **Power chain** — TPS63020 with D1 + C_SS soft-start on FB node; BQ24075 power-path
   (EN1/EN2/SYSOFF/nCE → STM32 GPIO outputs; nPGOOD/nCHG → STM32 GPIO inputs; TS → 10K to GND);
   LP103454-PCM-LD 2-wire battery connector (J3); 100K/100K battery voltage divider → PA1 ADC.

6. **USB-C (J1)** — single connector footprint, routed via USB3300 ULPI PHY (U8) to STM32
   OTG HS; CC1/CC2 5.1K pull-downs (R11/R12); USBLC6-2SC6 ESD (D2); USB3300 RST (PC1);
   24MHz TCXO (Y2); 12K RBIAS (R9); full decoupling.
   *(Base build: USB-C #2 + ULPI PHY fitted; USB-C #1 DNP. Both are HS-capable.)*

7. **IMU (LSM6DSOX/DSRX, U7)** — I2C4 (PD12/PD13); INT1 → PD14; INT2 → PD15.
   **Magnetometer (MMC5603NJ)** — sub-board via J11 (6-pin 0.1" header, I2C4 bus).

8. **SD card (J6)** — SDMMC1 4-bit bus (fixed AF pins above); card-detect → GPIO; well-decoupled VCC.

9. **STM32 backup domain (VBAT)** — wired directly to 3.3V rail; no coin cell.

10. **Config storage** — SD card file; no EEPROM fitted.

11. **OLED display (SSD1309)** — via J7 header; I2C4 bus (PD12 SCL, PD13 SDA); 0x3C/0x3D.
    On PCB v1.0. Driver bench-verified on F767ZI (task_display.c, ssd1309.c).

12. **Status LEDs** — LD3 (PD5, green) + LD4 (PD6, red): MCU open-drain + pull-ups R20/R21;
    LD1/LD2 hardwired to BQ24075 nPGOOD/nCHG.

13. **Mode-selection jumper** — PA0 GPIO input, internal pull-up; high = base, low = rover.

14. **BOOT0** — 2-pin header, 10K pull-down to GND; fitted = DFU bootloader.

15. **ESP32-S3 Zero (U5)** — spare STM32 UART; 3.3V power + decoupling; mode-select via
    PE2 (GP1) + PE3 (GP2/MODE_VALID). Base: WiFi NTRIP + BLE prov server.
    Rover: BLE GATT server to handheld.
