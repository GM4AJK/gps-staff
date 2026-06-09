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
| [BNO085-Datasheet.pdf](datasheets/BNO085-Datasheet.pdf) | CEVA/Bosch BNO085 9-DOF IMU datasheet (BNO08X DS 1000-3927 v1.17) -- the IMU specced for this project, used for pole lean correction. [DigiKey product page](https://www.digikey.co.uk/en/products/detail/ceva-technologies-inc/BNO085/9445940) |
| [STM32F765VIT6-Datasheet.pdf](datasheets/STM32F765VIT6-Datasheet.pdf) | STMicroelectronics STM32F765VIT6 MCU datasheet -- the MCU specced for this project (LQFP100, base+rover unified design). [DigiKey product page](https://www.digikey.co.uk/en/products/detail/stmicroelectronics/STM32F765VIT6/6137842) |
| [Core1262-LF-Schematic.pdf](datasheets/Core1262-LF-Schematic.pdf) | Waveshare Core1262-LF module schematic (410-510MHz variant) -- the LoRa module specced for this project (covers the 433/434MHz ISM sub-band and 70cm amateur band under GM4AJK licence). Sourced from the [Waveshare wiki](https://www.waveshare.com/wiki/Core1262-868M) (covers the LF/HF family). Note: not available on DigiKey -- ordered via Amazon; double-check the listing is the **LF** (410-510MHz) variant, not the 868M/HF (863-870MHz) variant which does not cover the required bands |
| [SX1262-Datasheet.pdf](datasheets/SX1262-Datasheet.pdf) | Semtech SX1262 LoRa transceiver chip datasheet -- the RF chip on the Core1262-LF module. Sourced from the [Waveshare wiki](https://www.waveshare.com/wiki/Core1262-868M). See also the [libdriver/sx1262](https://github.com/libdriver/sx1262) open-source driver library |
| [um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics-3.pdf](datasheets/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics-3.pdf) | STMicroelectronics UM1974 -- STM32 Nucleo-144 boards (MB1137) user manual. Reference for power supply options (Section 7.4), JP3/JP1 jumper configuration, and connector pinouts for the Nucleo-F767ZI bench board |
| [um2448-stlinkv3set-debuggerprogrammer-for-stm8-and-stm32-stmicroelectronics-1.pdf](datasheets/um2448-stlinkv3set-debuggerprogrammer-for-stm8-and-stm32-stmicroelectronics-1.pdf) | STMicroelectronics UM2448 -- ST-LINK V3SET debugger/programmer user manual. Reference for CN2/CN3 connector pinouts used when wiring to the Nucleo-F767ZI bench board |
