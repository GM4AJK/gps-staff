# Documents

A catalog of reference documents kept in this repo.

## Datasheets (`datasheets/`)

| Document | Description |
|---|---|
| [Nucleo-F767ZI-Schematic.pdf](datasheets/Nucleo-F767ZI-Schematic.pdf) | Schematic for the Nucleo-F767ZI dev board (STM32F7), used for firmware bring-up before the custom PCB |
| [ANN-MB-00-Datasheet.pdf](datasheets/ANN-MB-00-Datasheet.pdf) | u-blox ANN-MB-00 (UBX-18049862) GNSS antenna datasheet -- the antenna specced for this project. [DigiKey product page](https://www.digikey.co.uk/en/products/detail/u-blox/ANN-MB-00/9817928) |
| [ZED-F9P-05B-Datasheet.pdf](datasheets/ZED-F9P-05B-Datasheet.pdf) | u-blox ZED-F9P-05B GNSS module datasheet -- the GNSS module specced for this project. [DigiKey product page](https://www.digikey.co.uk/en/products/detail/u-blox/ZED-F9P-05B/24708221) |
| [TPS63020-Datasheet.pdf](datasheets/TPS63020-Datasheet.pdf) | TI TPS63020 buck-boost regulator datasheet -- the regulator specced for this project. Order code `TPS63020DSJR` (14-pin VSON exposed-pad, tape-and-reel) confirmed to match. [DigiKey product page](https://www.digikey.co.uk/en/products/detail/texas-instruments/TPS63020DSJR/2353761) |
| [BQ24075-Datasheet.pdf](datasheets/BQ24075-Datasheet.pdf) | TI BQ24075 power path management / battery charger datasheet -- the chip specced for this project. Order code `BQ24075TRGTR` (16-pin VQFN 3x3, tape-and-reel) confirmed to match. [DigiKey product page](https://www.digikey.co.uk/en/products/detail/texas-instruments/BQ24075TRGTR/2202252) |
| [STM32F765VIT6-Datasheet.pdf](datasheets/STM32F765VIT6-Datasheet.pdf) | STMicroelectronics STM32F765VIT6 MCU datasheet -- the MCU specced for this project (LQFP100, base+rover unified design). [DigiKey product page](https://www.digikey.co.uk/en/products/detail/stmicroelectronics/STM32F765VIT6/6137842) |
| [Core1262-LF-Schematic.pdf](datasheets/Core1262-LF-Schematic.pdf) | Waveshare Core1262-LF module schematic (410-510MHz variant) -- the LoRa module specced for this project (covers the 433/434MHz ISM sub-band and 70cm amateur band under GM4AJK licence). Sourced from the [Waveshare wiki](https://www.waveshare.com/wiki/Core1262-868M) (covers the LF/HF family). Note: not available on DigiKey -- ordered via Amazon; double-check the listing is the **LF** (410-510MHz) variant, not the 868M/HF (863-870MHz) variant which does not cover the required bands |
| [SX1262-Datasheet.pdf](datasheets/SX1262-Datasheet.pdf) | Semtech SX1262 LoRa transceiver chip datasheet -- the RF chip on the Core1262-LF module. Sourced from the [Waveshare wiki](https://www.waveshare.com/wiki/Core1262-868M). See also the [libdriver/sx1262](https://github.com/libdriver/sx1262) open-source driver library |
| [um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics-3.pdf](datasheets/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics-3.pdf) | STMicroelectronics UM1974 -- STM32 Nucleo-144 boards (MB1137) user manual. Reference for power supply options (Section 7.4), JP3/JP1 jumper configuration, and connector pinouts for the Nucleo-F767ZI bench board |
| [um2448-stlinkv3set-debuggerprogrammer-for-stm8-and-stm32-stmicroelectronics-1.pdf](datasheets/um2448-stlinkv3set-debuggerprogrammer-for-stm8-and-stm32-stmicroelectronics-1.pdf) | STMicroelectronics UM2448 -- ST-LINK V3SET debugger/programmer user manual. Reference for CN2/CN3 connector pinouts used when wiring to the Nucleo-F767ZI bench board |
| [SSD1309.pdf](datasheets/SSD1309.pdf) | Solomon Systech SSD1309 OLED display controller datasheet -- the controller for the I2C1 OLED module being added to the Nucleo-F767ZI sandbox |
| [LSM6DSOX.pdf](datasheets/LSM6DSOX.pdf) | STMicroelectronics LSM6DSOX accelerometer + gyroscope datasheet -- the IMU specced for tilt compensation and the bubble-level display, replacing the BNO085 |
| [LSM6DSRX-Datasheet.pdf](datasheets/LSM6DSRX-Datasheet.pdf) | STMicroelectronics LSM6DSRX accelerometer + gyroscope datasheet -- pin-compatible (LGA-14L) alternative to the LSM6DSOX with the same Machine Learning Core/FSM, considered after the LSM6DSOXTR went out of stock. WHO_AM_I reads 0x6B instead of the LSM6DSOX's 0x6C |
| [adafruit_LSM6DSOX_BOB.png](datasheets/adafruit_LSM6DSOX_BOB.png) | Adafruit LSM6DSOX breakout (#4438) schematic -- shows the I2C/SPI signals and address strapping for the breakout board |
| [um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics-4.pdf](datasheets/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics-4.pdf) | STMicroelectronics UM1724 -- STM32 Nucleo-64 boards (MB1136) user manual. Reference for power supply options (Section 7.5), JP5/JP6 (IDD) jumper configuration, and connector pinouts for the Nucleo-F446RE/G491RE bench boards |

## BNO085 (`datasheets/bno085/`)

Out-of-spec side project (not part of the LSM6DSOX+LIS3MDL tilt/azimuth design) -- restored from history for a fresh attempt at the BNO085 over I2C instead of SPI.

| Document | Description |
|---|---|
| [BNO085-Datasheet.pdf](datasheets/bno085/BNO085-Datasheet.pdf) | CEVA/Bosch BNO085 9-DOF IMU datasheet (BNO08X DS 1000-3927 v1.17). [DigiKey product page](https://www.digikey.co.uk/en/products/detail/ceva-technologies-inc/BNO085/9445940) |
| [Adafruit-BNO085-BOB-Schematic.png](datasheets/bno085/Adafruit-BNO085-BOB-Schematic.png) | Adafruit BNO085 9-DOF breakout (#4754, rev C) schematic -- shows the level-shifted SPI/I2C signals (SCK, MOSI, MISO, CS, INT) and P0/P1/BOOTP strapping options. Sourced from the [Adafruit Learning Guide](https://learn.adafruit.com/adafruit-9-dof-orientation-imu-fusion-breakout-bno085) |
| [SH-2-Reference-Manual.pdf](datasheets/bno085/SH-2-Reference-Manual.pdf) | CEVA SH-2 Reference Manual (1000-3625) -- defines the SH-2 sensor hub protocol used over SHTP by the BNO085: Set/Get Feature commands, the full sensor report ID table (e.g. `0x05` Rotation Vector, `0x08` Game Rotation Vector, `0x09` Geomagnetic Rotation Vector), and input report payload layouts (quaternion scaling, accuracy fields, etc.). Sourced from [ceva-ip.com](https://www.ceva-ip.com/wp-content/uploads/SH-2-Reference-Manual.pdf) |
