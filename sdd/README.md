# GPS Survey Staff -- Project Context

## Overview

A DIY RTK GNSS survey staff built around the u-blox ZED-F9P module on a custom carrier PCB.
The system consists of two identical PCBs -- one configured as base station, one as rover --
communicating via LoRa. Centimetre-level relative accuracy is the target.

## Document Purpose

This document is a pre-design requirements and decisions capture, maintained as a living reference throughout the research and specification phase. It is intended to serve as the primary input to a formal OpenSpec SDD (System Design Document) workflow when hardware and software design phases begin. All component decisions, rationale, interface definitions, and open items recorded here are structured to map directly into SDD sections.

---

## Development Tools Available

- Nucleo-F767ZI -- STM32F7 development board for firmware bring-up before PCB
- UNI-T UDP3305S -- professional programmable bench PSU (current limiting for safe bring-up)
- Elesys USB Explorer -- USB protocol analyser (enumeration verification, CDC/MSC composite debugging, 100mA->500mA current transition validation)
- Hot plate / hot air reflow station
- 3D printer (enclosure prototyping)

---

## Designer Background

- Electrical engineer
- Licensed amateur radio operator: **GM4AJK** (Scotland)
- RF and antenna design experience -- not a concern for this project
- KiCad for EDA
- Hot plate / hot air reflow capability
- 3D printer for enclosure design

---

## Key Technology Decisions

### GNSS Module: u-blox ZED-F9P

- The ZED-F9P is a **module** (LCC package, 17 x 22 mm), not a bare chip
- Contains: F9 chipset, RF front end, internal TCXO, flash -- all internal
- Carrier board design is the user's task (analogous to what ArduSimple produce)
- Supports RTK processing **onboard** -- outputs corrected NMEA/UBX positions
- Supports multi-constellation: GPS, GLONASS, Galileo, BeiDou
- **Variant: ZED-F9P-05B** (L1/L2) -- more mature ecosystem, better L2 satellite availability, aligns with most NTRIP infrastructure

### MCU: STM32F765VIT6

- 100-pin LQFP (14x14mm, 0.5mm pitch) -- ~5000 units in stock at DigiKey
- 2MB internal flash
- 512KB RAM
- 216MHz Cortex-M7 with FPU
- All required peripherals available: UART (F9P), 2x SPI (SD card + LoRa), USB FS, SWD
- Same MCU on both base and rover (single unified PCB design)
- Same STM32F7 family as Nucleo-F767ZI dev board -- firmware port to PCB is pin reassignment only
- Rejected alternatives:
  - STM32G431KB: 128KB flash too tight; 32KB SRAM insufficient
  - STM32H7A3RIT6 (LQFP64): preferred on pin count but no stock until 2027
  - STM32H750VBT6: requires external QSPI flash chip, adds complexity
  - STM32H743VIT6: overspecced (RTKLIB no longer running on MCU); F9P handles RTK onboard; F765 gives identical capability with simpler HAL and direct Nucleo firmware portability
- Note: LQFP100 vs LQFP64 makes negligible soldering difference -- same 0.5mm pitch, same hot air technique

### Communications: LoRa

- ZigBee was initially considered but ruled out on range grounds
- **Module: Waveshare Core1262-LF** (SX1262, 410-510MHz)
- 22dBm output, -148dBm sensitivity -- massively over-specified for 100m working range
- SPI interface, 18MHz max, BUSY pin must be checked low before every transaction
- Operating frequency set in software (SetRfFrequency):
  - Default: ISM no-duty-cycle sub-band 434.040-434.790 MHz (10mW limit, ~2.5% duty cycle at 1Hz/SF7/BW500)
  - Fallback: 70cm amateur band under GM4AJK licence (full 22dBm, no duty cycle restriction)
- LoRa settings: SF7, BW500 -- ~25ms time-on-air per 700-byte RTCM packet
- **Bidirectional half-duplex protocol** -- SX1262 is a transceiver, both units TX and RX
  - Primary: base -> rover RTCM stream at 1Hz
  - Secondary: rover -> base command channel in gaps between RTCM packets
  - Rover commands: HELLO (link test), START_SURVEY_IN, STATUS_REQUEST
  - Base responses: STATUS (survey-in progress/variance/time, fixed mode, idle), BASE_POSITION
  - Startup sequence: rover sends HELLO -> base responds STATUS -> rover triggers survey-in if needed -> rover displays survey-in progress -> base enters fixed mode -> RTCM stream begins
  - User never needs physical access to base station after placement -- fully remote controlled from rover

### Antenna

- **u-blox ANN-MB-00** (UBX-18049862) -- designed to pair with u-blox F9P modules -- [DigiKey Link](https://www.digikey.co.uk/en/products/detail/u-blox/ANN-MB-00/9817928), Mass Production status
- Chosen for mainstream availability: stocked by DigiKey and Mouser, no single-vendor risk
- Bands: L1 (1559-1606 MHz) and L2/E5b/B2I (1197-1249 MHz) -- covers F9P-05B (L1/L2 used)
- Architecture: dual hybrid couplers (RHCP), dual SAW pre-filter per band, dual LNA chain -- high out-of-band rejection
- Patch antenna gain: L1 = 3.5 dBic typ (zenith), L2 = 0.0-2.0 dBic typ (zenith)
- Axial ratio: max 2.0 dB at zenith (both bands) -- good RHCP purity
- LNA gain: 28 +/- 3.0 dB per band
- LNA noise figure: max 2.8 dB (L1), max 3.2 dB (L2)
- Cable: RG-174, 5.0 m permanently attached, insertion loss 6.6 dB typ at L1
- Total gain (LNA minus cable loss): 21.4 dB typ (L1), 21.4-22.4 dB typ (L2)
- Out-of-band rejection: >65 dB at cellular bands -- good urban/suburban performance
- **LNA supply: 3.0-5.0V, 15 mA typical** -- single supply for both bands, 3.3V bias-T compatible
- Phase center offset (ref ARP): L1 < 5mm horizontal / 8.9 mm up; L2 < 5mm horizontal / 7.6 mm up
- Phase center variation over azimuth/elevation: L1 < 5mm, L2 < 10mm
- IP67 (temporary immersion to 1m) -- exceeds project requirement
- Operating temperature: -40 to +85C
- Cable termination: **SMA plug (male)** -- integral pigtail, no body connector
- PCB connector: **SMA female through-hole** -- cable plugs directly in, no adaptor needed
- RF path: 50 ohm controlled-impedance trace from F9P RF_IN pad to SMA
- Mounting: magnetic base or 2x M4 screw (survey pole compatible)
- Separate SMA for LoRa 433MHz antenna (quarter-wave monopole ~164mm or helical whip)

### Power Supply

- **Battery**: LP103454-PCM-LD Li-ion Polymer, 3.7V nominal, 2000mAh
  - Physical dimensions: 10mm x 34mm x 54mm -- drives enclosure design
  - 2-wire (positive + negative only, no thermistor wire)
  - Integrated PCM (Protection Circuit Module): overcharge, overdischarge, overcurrent protection built in
  - Estimated runtime: ~5 hours at 350mA typical draw (TPS63020 at ~90% efficiency)
  - BQ24075 TS pin: fit fixed 10K resistor from TS to GND -- mimics NTC at 25C, keeps charger in valid range, temperature monitoring effectively disabled (PCM provides thermal protection independently)
- **Prototype**: Bench PSU -- UNI-T UDP3305S (professional programmable unit, current limiting for safe bring-up)
- **Final design**: Integrated on PCB
- **Regulator**: TPS63020 (TI) buck-boost -- 1.8-5.5V in, up to 2A, stable 3.3V across full LiPo discharge curve
  - LDO ruled out -- loses 3.3V rail below ~3.5V input, wasting usable battery capacity
  - **Soft-start / inrush current**: TPS63020 can exhibit a soft-start trip on startup due to output capacitor inrush current
    - Mitigation: add a small switching diode (D1) and soft-start capacitor (C_SS) directly to the FB node
    - C_SS holds FB low at startup, causing the regulator to ramp output slowly -- limits inrush current
    - D1 allows C_SS to discharge rapidly on load transients so normal regulation is unaffected
    - C_SS value sets startup ramp time -- larger value = slower ramp = lower inrush
    - Reference: TI application note SLVA553 (https://www.ti.com/lit/an/slva553/slva553.pdf)
- **USB-C**: 5V charge/power port
  - No PD negotiation needed for basic 5V charging
  - CC1 + CC2 pulled to GND via 5.1k ohm resistors identifies unit as 5V sink
- **Power path management**: BQ24075 -- allows simultaneous USB-powered operation + battery charging, no glitches on hot-plug
  - TI WEBENCH / reference designs available for both TPS63020 and BQ24075
  - BQ24075 control pins wired to STM32 GPIOs (spare pins available on LQFP100):
    - EN1, EN2: input current limit select -- 00=100mA, 10=500mA, 01=suspend; firmware switches 100mA->500mA after USB enumeration
    - SYSOFF: pull high to disconnect battery from system output -- software shutdown / shipping mode
    - nCE: active low charge enable -- firmware can disable charging (thermal event, USB absent, etc.)
  - Hardware defaults via 10K pull-down resistors on all four control pins:
    - EN1=0, EN2=0: 100mA USB100 mode at reset/boot -- USB compliant before enumeration
    - SYSOFF=0: normal operation (battery connected) at reset
    - nCE=0: charging enabled at reset
    - MCU overrides by driving pins HIGH after initialisation (10K pull-down easily overcome by GPIO output)
- **Battery voltage monitor**: resistor divider from VBATT to GND, midpoint to STM32 ADC pin
  - R_top = 100K, R_bottom = 100K (1% tolerance) -- midpoint = VBATT/2
  - ADC reads VBATT/2; firmware doubles reading to get VBATT, maps to battery %
  - LiPo range: 3.0V (0%) to 4.2V (100%) -- midpoint 1.5V to 2.1V, well within STM32 ADC range
  - Divider current: ~4.2V / 200K = ~21uA -- negligible quiescent drain
- **Estimated current draw**: ~300-400 mA typical survey (F9P ~50mA, ANN-MB LNA ~15mA via bias-T, STM32F7 ~100mA, LoRa TX ~120mA, SD ~100mA peak, display ~10mA, BNO085 ~6mA, buzzer ~20mA when active, USB3300 ~0.1mA suspend)
  - USB file transfer mode (rover): add ~55mA for USB3300 active + SD read current

### IMU: BNO085

- Bosch/CEVA BNO085 -- 9-DOF (accelerometer + gyroscope + magnetometer) with onboard CEVA SH-2 fusion processor
- Purpose: pole lean correction with full azimuth -- calculates corrected ground point position when pole is not perfectly vertical
- Chosen over LSM6DSO (6-DOF): onboard fusion outputs calibrated orientation directly, automatic hard/soft iron magnetometer calibration, no filter implementation required in firmware
- Datasheet: BNO08X DS 1000-3927 v1.17 -- https://www.ceva-ip.com/wp-content/uploads/BNO080_085-Datasheet.pdf
- Package: 28-pin LGA, 3.8x5.2x1.1mm -- SMD, hand-solderable with hot air
- Supply: VDD 2.4-3.6V (sensors), VDDIO 1.65-3.6V (I/O) -- both 3.3V rail compatible
- Current draw: ~6.1mA typical (fusion running) -- negligible in power budget
- Interface: **SPI** selected via PS1=1, PS0=1 (both pins pulled high at reset)
  - Switched from I2C: STM32F765 ULPI + SDMMC1 pin conflicts leave only one I2C bus available,
    needed for display + EEPROM. SPI also eliminates BNO085 I2C protocol violation risk entirely.
  - SPI pins: SCL=SCK, SDA=MISO, DI=MOSI, H_CSN=CS (active low)
  - INT and RST pins both required for stable SPI operation (per Adafruit)
  - I2C bus (single available): I2C1 = SSD1309 display (0x3C) + AT24C04 EEPROM (0x50) only
- Communication: SHTP (Sensor Hub Transport Protocol) over I2C -- open-source libraries available (Adafruit BNO08x, community STM32 ports)
- Key pin connections to STM32:
  - NRST (pin 11): active low reset -> STM32 GPIO -- required for stable SPI operation
  - H_INTN (pin 14): interrupt -> STM32 GPIO -- required for stable SPI operation
  - PS1 (pin 5): pulled high (SPI mode)
  - PS0/WAKE (pin 6): pulled high (SPI mode); repurposed as WAKE after reset in SPI mode
  - BOOTN (pin 4): pulled high (normal operation)
  - CAP (pin 9): 100nF to GND -- required external capacitor
- Clock: internal oscillator -- no external crystal required
- Development: Adafruit BNO085 breakout board -- set P0/P1 solder jumpers high for SPI mode (default is I2C)
- Outputs used: gravity vector report (tilt), rotation vector (absolute orientation, magnetically referenced)
- Automatic calibration: hard/soft iron distortion compensated internally -- improves continuously during use
- Note: BNO086 is a drop-in replacement (identical pinout, same software) with lower idle power and 14-bit accelerometer fusion -- consider at ordering time

**Lean correction algorithm:**

```
gravity = BNO085_read_gravity_vector()          // calibrated, filtered -- no raw ax/ay/az needed

tilt_angle = arccos(gravity.z / |gravity|)      // angle from vertical
tilt_azimuth = arctan2(gravity.y, gravity.x)    // magnetic north referenced -- full azimuth correction

offset = pole_height x sin(tilt_angle)          // horizontal offset of ground point

corrected_lat = gnss_lat + (offset x cos(tilt_azimuth + 180)) / earth_radius
corrected_lon = gnss_lon + (offset x sin(tilt_azimuth + 180)) / (earth_radius x cos(gnss_lat))
```

- pole_height: configurable parameter stored in EEPROM -- antenna phase center (ARP) to ground contact point
  - Entered via startup prompt at every boot (encoder to adjust, press to confirm, 10s timeout accepts stored value)
  - Applied to both horizontal lean correction and vertical height correction:
    ground_height = gnss_height - (pole_height x cos(tilt_angle))
- Bubble level display dot driven from gravity.x / gravity.y -- pre-filtered, no additional processing
- Lean angle and correction magnitude shown on display
- At 2m pole height: 1 degree lean = 35mm uncorrected error; BNO085 correction reduces residual to ~3.5mm

### Data Logging -- SD Card

- MicroSD via **SDMMC1 4-bit mode** -- native SD interface, not SPI
  - Replaces earlier SPI mode decision (made before MCU was finalised)
  - STM32F765 has two SDMMC peripherals -- using SDMMC1
  - 4-bit bus + 50MHz clock = ~20-25 MB/s practical throughput vs ~3 MB/s SPI
  - Hardware CRC checking built in -- more robust than SPI mode
  - Dedicated peripheral -- LoRa SX1262 gets its own SPI bus (no sharing)
  - SD write rate at 1Hz logging is trivially low for any interface -- SDMMC performance chosen for the USB MSC read path
  - On USB MSC file transfer: SDMMC 4-bit (~20 MB/s) ensures SD card never starves the USB pipeline
  - Actual bottleneck is USB OTG FS (~1 MB/s at Full Speed 12 Mbps) -- SDMMC comfortably feeds it
  - USB HS (~40 MB/s) would require external ULPI PHY -- complexity not justified for this project
  - Fixed alternate function pins on STM32F765: PC8(D0), PC9(D1), PC10(D2), PC11(D3), PC12(CLK), PD2(CMD)
- Connector: GCT MEM2055 or Amphenol 101-00303 (push-push microSD)
- Card detect (CD) pin wired to STM32 GPIO
- Decouple VCC well -- SD cards spike current during writes
- Firmware: Elm Chan FatFS + STM32 HAL SDMMC driver with DMA -- well supported by ST
- Log format: raw NMEA and/or UBX binary per session + relative X,Y,Z positions from datum
- File timestamps derived from F9P GNSS time via 1PPS signal on STM32 GPIO
- Data transferred to laptop for import into FreeCAD or post-processing

### STM32F765 Backup Domain (VBAT)

- STM32F765 VBAT pin powers the backup domain independently of main VDD
- Backup domain preserved when main power is off, as long as VBAT has supply
- **VBAT supply**: 3.3V rail via Schottky diode -- LiPo always installed in enclosure so backup domain always retained
  ```
  3.3V ---[Schottky]--- VBAT ---[100nF]--- GND
  ```
- Backup domain current: ~1-2uA -- negligible drain on LiPo
- No coin cell required -- LiPo provides continuous VBAT when installed

**Backup Registers:**
- 32 x 32-bit registers = 128 bytes -- small flags, timestamps, last-known values

**Backup SRAM: 4KB**
- Preserved across power cycles as long as LiPo installed
- Requires: enable backup SRAM clock + disable write protection in firmware
- Planned usage:
  - Datum point coordinates (lat, lon, height = ~24 bytes)
  - Survey session state (points recorded, session timestamp)
  - BNO085 calibration state
  - Last known GNSS position (aids F9P convergence on next power-on)
  - Mirror of critical EEPROM config for fast boot access

### Configuration Storage -- I2C EEPROM

- **AT24C04 or M24C04** -- 4Kbit (512 bytes) I2C EEPROM
- Shares I2C bus with SSD1309 display -- no additional wiring (IMU is on SPI, not I2C)
- I2C address: 0x50-0x57 (set via A0/A1/A2 pins) -- no conflict with display (0x3C)
- Supply: 1.8-5.5V -- 3.3V rail compatible
- Write endurance: ~1,000,000 cycles -- effectively unlimited for config use
- Byte-level write, no sector erase, no interrupt management required
- Package: SOT-23-5 or SOIC-8 -- standard hand-solderable SMD
- Driver: trivial I2C byte read/write, no library dependency

**Config data stored (~50-100 bytes):**
- Buzzer enable (1 byte)
- Pole height in mm (4 bytes, float -- feeds lean correction and height subtraction; prompted at every boot)
- LoRa band preference: ISM or amateur (1 byte)
- Logging preferences (1 byte)
- Reserved/future expansion

### Remote Trigger Input

- External trigger connector for a remote momentary push button mounted on the survey pole
- Allows surveyor to mark a geo-point without touching the rover controls -- eliminates vibration/movement during measurement
- **Connector: 3.5mm stereo jack** (panel mount on enclosure)
  - Standard extension cables universally available and cheap
  - Surveyor can wire any momentary NO switch to a 3.5mm plug
  - Weatherproof 3.5mm jack variants available
  - Tip or ring shorted to sleeve (GND) on button press -- firmware reads either
- GPIO: STM32 input with internal pull-up -- button press pulls line to GND
- Hardware debounce: 100nF capacitor + 10K resistor RC filter on GPIO input
- Firmware debounce: software confirmation in addition to hardware filter
- Function: identical to the "record geo-point" front panel button -- parallel input

### Buzzer

- Passive piezo buzzer -- PWM driven via STM32 TIM channel
- Chosen over active buzzer: variable frequency allows distinct tones per event
  - Short pip: geo-point recorded
  - Double pip: warning (SD full, LoRa link lost, etc.)
  - Long tone: RTK fix lost
- Drive circuit: STM32 PWM GPIO -> 1k resistor -> 2N7002 MOSFET gate; buzzer between drain and 3.3V; source to GND
- Audio feedback on/off: software flag, user-configurable via encoder menu, stored in STM32 flash
- Outdoor use -- select buzzer rated 80dB+ at 10cm for audibility in wind

### Enclosure

- Custom 3D printed design

---

## System Architecture

```
BASE STATION                         ROVER
+------------------+                 +-------------------+
|  F9P (base mode) |                 |  F9P (rover mode)  |
|  generates RTCM  |                 |  receives RTCM     |
|  on UART out     |                 |  solves RTK        |
+--------+---------+                 |  outputs NMEA/UBX  |
         | UART                      +--------+-----------+
+--------v---------+  LoRa RTCM -> +--------v-----------+
|   STM32F7        | <- LoRa CMD   |   STM32F7           |
|   STM32F7        |               |   STM32F7           |
|   reads RTCM     |               |   feeds RTCM->F9P   |
|   forwards via   |               |   logs position     |
|   LoRa           |               |   to SD card        |
+------------------+               +---------------------+
```

- Single PCB design, identical firmware flashed to both units, mode (base/rover) selected by hardware jumper read at boot
- F9P handles all RTK computation onboard -- STM32F7 is smart pipe + data logger
- RTCM data rate: ~5 kbps for 4 constellations at 1 Hz -- well within LoRa capability

---

## RTK Concepts Established

- **Code-phase** (consumer GPS): ~1-5 m accuracy -- noise floor cannot be corrected below ~1 m
- **Carrier-phase** (RTK): ~mm measurement noise -- RTK resolves integer ambiguity to get cm positions
- **DGPS** corrects systematic errors but cannot get below code-phase noise floor
- **RTK base position**: only affects *absolute* accuracy, not *relative* accuracy between points
  - Unknown base position -> cm relative accuracy, offset absolute position
  - Known base position (benchmark / NTRIP) -> cm absolute accuracy
- **NTRIP**: UK correction networks available for absolute coordinate accuracy (OS datum)

---

## Open Questions / TBD

- [x] ZED-F9P-05B selected (L1/L2)
- [x] STM32F765VIT6 selected as MCU (LQFP100, ~5000 units in stock at DigiKey)
- [x] LoRa module selected: Waveshare Core1262-LF (SX1262, 410-510MHz, covers 70cm amateur band)
  - Used for both prototype and final design -- no swap needed
  - 22dBm output power, -148dBm sensitivity
  - SPI interface, 18MHz max clock
  - BUSY pin must be checked low before every SPI transaction (SX1262 requirement)
  - Frequency set in software via SetRfFrequency SPI command -- no hardware change needed to switch bands:
    - ISM (no duty cycle sub-band): 434.040-434.790 MHz, 10mW max
    - 70cm amateur (GM4AJK licence): 430-440 MHz, full 22dBm, no duty cycle restriction
  - Default: start on ISM sub-band; switch to amateur band in firmware if required
  - LoRa antenna is separate from GNSS antenna -- 433/434 MHz quarter-wave monopole (~164mm) or helical whip
- [x] Antenna connectors: SMA female through-hole (2 off -- GNSS and LoRa)
  - GNSS: 50 ohm controlled-impedance trace from F9P U.FL pad to edge-mount SMA
  - LoRa: SMA to Core1262-LF antenna port
  - Through-hole SMA preferred over SMD edge-mount for mechanical robustness on handheld device
- [x] Buck-boost regulator: TPS63020 confirmed
- [ ] LiPo battery capacity -- deferred until real power consumption measured from prototype boards; will also drive 3D printed enclosure dimensions

## Pre-Schematic Decisions Needed

- [x] Mode selection: hardware jumper read at boot
  - GPIO pin with pull-up/pull-down, sampled once at startup
  - Jumper fitted = base station mode; jumper absent = rover mode (or vice versa -- to be decided at schematic)
  - Identical firmware binary flashed to both units -- no separate builds
  - All base/rover code paths compiled in; mode flag gates behaviour at runtime
- [x] User input: 4 x momentary push buttons (functions TBD) + 1 x quadrature rotary encoder with integrated push button
  - Encoder: KY-040 module (EC11-type quadrature encoder with knob cap), A/B outputs wired to STM32 TIMx CH1/CH2 in encoder mode (TIM_ENCODERMODE)
  - Encoder push button (SW): additional GPIO input, treated as a 5th button
  - Total button inputs: 5 (4 discrete + 1 encoder push)
  - KY-040 module has onboard 10k pull-ups on CLK, DT, SW -- final PCB using bare EC11 component must add these pull-ups
  - Button functions undefined at this stage -- to be assigned during firmware development
- [x] Status LEDs: 3 x LEDs, all driven from STM32 GPIO outputs via current-limiting resistors
  - Functions TBD during firmware development -- MCU has full control of patterns (solid, blink, combined status)
  - Independent of display -- visible when display is off or sleeping
  - BQ24075 status inputs to STM32 GPIO (MCU reads these to decide LED behaviour):
    - nPGOOD -> STM32 GPIO input + 10K pull-up to 3.3V (open drain, active low = power good)
    - nCHG -> STM32 GPIO input + 10K pull-up to 3.3V (open drain, active low = charging in progress)
  - **HARDWARE REQUIREMENT**: all LED GPIO pins must be assigned to PWM-capable pins (TIMx_CHx) on STM32F765 -- verify during KiCad pin assignment
  - **FIRMWARE NOTE (OpenSpec SDD phase)**: develop a PWM LED driver module abstracting N attached LEDs
    - Module handles: brightness (PWM duty cycle), blink patterns, fade effects
    - Caller simply sets LED state -- driver handles timing and PWM independently
    - Design for N LEDs so adding/removing LEDs requires no driver changes
- [x] F9P configuration: sent via UART on each boot
  - STM32 sends UBX-CFG messages to F9P over UART at startup before entering normal operation
  - No dependency on F9P internal flash -- config is always authoritative from firmware
  - Mode-dependent config (base vs rover) sent according to jumper state read at boot
- [x] BOOT0: 2-pin jumper header
  - Jumper absent (BOOT0 pulled low): normal boot from flash
  - Jumper fitted (BOOT0 pulled high): STM32 built-in DFU bootloader -- firmware update via USB-C without SWD programmer
  - 10k pull-down resistor to GND ensures defined state when jumper absent
- [x] Display: Podazz 2.42" SSD1309 128x64 OLED via I2C (https://www.amazon.co.uk/Podazz-SSD1309-128x64-OLED-Display/dp/B0FD9XFVMV/)
  - Chosen over 0.96" SH1106 (used in previous dog scales project) for readability -- 2.5x physical area, same resolution
  - Interface: I2C, address 0x3C/0x3D -- same bus and address range as SH1106, no PCB wiring changes
  - Driver port from SH1106: remove column +2 offset, update init sequence (SSD1306-compatible base) -- framebuffer and layout code unchanged
  - SH1106 available as fallback for prototype bring-up (driver already written)
  - Dedicated I2C bus on STM32F765 (avoid sharing with F9P UART path)
  - Layout: split screen -- left panel bubble level, right panel data fields
    - Left ~40x48px: circular bubble level (Bresenham circle, filled dot)
      - Dot position maps directly to LSM6DSO ax/ay: dot_x = cx + (ax/g x scale), dot_y = cy + (ay/g x scale)
      - Scale factor sets tilt angle at full deflection (e.g. 5 degrees = circle edge)
      - Inner ring indicates acceptable tilt threshold
      - Dot clamped to circle boundary when tilt exceeds display range
    - Right panel data fields: mode (base/rover), RTK fix status, satellite count, HDOP, tilt angle (degrees numeric), battery %, SD status/recording, LoRa link quality
- [x] USB-C interface -- two connector footprints on PCB, one fitted per build
  - **USB-C #1 (base station build)**: D+/D- routed to STM32 OTG FS (PA11/PA12) -- internal PHY, 12 Mbps FS
    - Base station power-only use: VBUS charging + EN1/EN2 current negotiation
    - Internal D+ pull-up on STM32F765 -- no external 1.5k ohm required
  - **USB-C #2 (rover build)**: D+/D- routed to external ULPI PHY -- STM32 OTG HS, 480 Mbps
    - ULPI PHY: **USB3300** (Microchip) -- well documented, widely used with STM32, QFN-32 (5x5x0.9mm)
    - Datasheet: DS00001783C -- https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/00001783C.pdf
    - TinyUSB STM32 OTG HS + ULPI driver supported
    - Rover use: USB HS CDC + MSC composite
    - Supply: 3.3V single rail -- internal 1.8V regulators enabled via REG_EN pin, no external 1.8V needed
    - Current: 54.7mA typical active (USB connected), 83uA suspend (USB disconnected during survey)
    - ULPI interface to STM32: CLK, STP, DIR, NXT, DATA[7:0] -- 12 pins total
    - USB side: DP, DM to USB-C connector; VBUS monitored internally
    - Requires 24MHz crystal on XI/XO pins -- dedicated crystal, additional PCB component
    - RBIAS: 12K +/-1% resistor to GND -- mandatory
    - GND flag: via array to ground plane -- critical, main IC ground connection
    - Decoupling: 0.1uF on each VDD3.3 pin (x4); 0.1uF + 4.7uF low-ESR on VDD1.8/VDDA1.8
    - Device-only config (not OTG host): ID->GND, CPEN->GND, EXTVBUS->GND
    - RESET pin: connect to STM32 GPIO (has internal pull-down, firmware can reset PHY)
  - VBUS and GND common to both connectors -- same BQ24075 power circuit serves either
  - CC1 + CC2: 5.1k ohm to GND per connector footprint -- only fitted connector's CC resistors active
  - ESD protection: USBLC6-2SC6 per connector footprint -- DNP with connector
  - VBUS -> BQ24075 input + GPIO (voltage divider) for USB presence detection
  - USB stack: TinyUSB (not ST CubeMX middleware -- cleaner composite device support)
    - STM32F7 supported via TinyUSB DWC2 driver (same driver as Nucleo-F767ZI dev board)
    - STM32 port: https://github.com/lbthomsen/tinyusb
  - Composite CDC + MSC (rover):
    - CDC: NMEA/UBX streaming to PC, RTKLIB connection, config commands -- no PC drivers needed
    - MSC: SD card exposed as USB mass storage drive for log file transfer -- HS gives ~30-40 MB/s
    - SD card exclusive access: on USB connect -> pause logging, unmount FatFS, hand to MSC
                                on USB disconnect -> remount FatFS, resume logging
  - USB DFU: firmware updates via STM32 built-in bootloader (BOOT0 pin), independent of TinyUSB
  - DNP assembly split:
    - Base station: USB-C #1 fitted, USB-C #2 DNP, ULPI PHY DNP
    - Rover: USB-C #2 fitted, USB-C #1 DNP, ULPI PHY populated
- [x] Expected working range: 100m
  - Path loss at 100m/433MHz: ~65 dB; link budget margin: ~105 dB -- module massively over-specified but already chosen
  - Use SF7, BW500 (fastest settings): ~25ms time-on-air per 700-byte RTCM packet
  - At 1Hz: ~2.5% duty cycle -- within ISM no-duty-cycle sub-band (434.040-434.790 MHz)
  - Amateur licence remains backstop but ISM band handles normal operation at these settings
- [x] PCB: 4-layer, manufactured by JLCPCB
  - Stackup: JLC04161H-7628 (standard JLCPCB 4-layer)
    - L1 (Top): components, signal, RF traces
    - L2: solid GND plane (RF reference)
    - L3: 3.3V power plane
    - L4 (Bottom): secondary signal
  - 50 ohm microstrip on L1 (L2 GND reference): ~0.7mm trace width
  - Use JLCPCB controlled impedance option for RF traces to SMA connectors

---

## Reference Documents

- [TI SLVA553] TPS63020 soft-start / inrush current mitigation -- diode + C_SS on FB node
  https://www.ti.com/lit/an/slva553/slva553.pdf

- [TI SLUA901] BQ24040 Application Report (June 2018) -- charge current resistor values, termination, circuit topology
  Note: BQ24040 is a close relative of the specced BQ24075 -- lacks integrated power path but topology and R-ISET values apply directly
  https://www.ti.com/lit/an/slua901/slua901.pdf

- [TI Datasheet] BQ24075-Q1 datasheet -- automotive grade variant of the specced BQ24075, functionally identical for this application
  https://www.ti.com/lit/ds/symlink/bq24075-q1.pdf

- [Microchip DS00001783C] USB3300 datasheet -- Hi-Speed USB ULPI PHY, 32-pin QFN
  https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/00001783C.pdf

- [CEVA 1000-3927 v1.17] BNO08X datasheet -- 9-DOF IMU with onboard SH-2 fusion, 28-pin LGA
  https://www.ceva-ip.com/wp-content/uploads/BNO080_085-Datasheet.pdf
