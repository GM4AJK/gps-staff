# SX1262 Quick Reference

Cheat-sheet for the values needed to set the SX1262's TX power, frequency,
and LoRa bandwidth/SF/CR without re-reading `docs/datasheets/SX1262-Datasheet.pdf`
every time. All driver functions referenced here are in
`Core/Src/sx1262.c` / `Core/Inc/sx1262.h` (identical on both boards).

## TX Power -- `sx1262_set_pa_config()` + `sx1262_set_tx_params()`

Two steps: `sx1262_set_pa_config(p, paDutyCycle, hpMax, device_sel)` sets
the **PA ceiling** (max achievable power), then
`sx1262_set_tx_params(p, power_dbm, ramp_time)` sets the **actual output
power** (must be <= the PA ceiling).

For SX1262, always use `device_sel = SX1262_PA_CONFIG_SX1262` (high power
PA, `deviceSel = 0x00`).

### PA ceiling presets (datasheet Table 13-21, SX1262 rows)

| Ceiling | paDutyCycle | hpMax |
|---|---|---|
| +22 dBm (max) | 0x04 | 0x07 |
| +20 dBm | 0x03 | 0x05 |
| +17 dBm | 0x02 | 0x03 |
| +14 dBm | 0x02 | 0x02 |

### `sx1262_set_tx_params()` power range

With the high-power PA selected (`SX1262_PA_CONFIG_SX1262`), `power_dbm`
is **-9 to +22 dBm in 1dB steps**, clipped at whatever the PA ceiling
above allows. To get an exact output power below a ceiling, just pass
that dBm value directly -- e.g. PA ceiling +14dBm + `set_tx_params(p, 10, ...)`
gives ~+10dBm (~10mW) output.

Common dBm <-> mW: 0dBm=1mW, 10dBm=10mW, 14dBm=25mW, 17dBm=50mW,
20dBm=100mW, 22dBm=158mW.

`ramp_time`: one of `SX1262_RAMP_10U` .. `SX1262_RAMP_3400U` (10us-3.4ms).
`SX1262_RAMP_200U` has been used so far.

## Frequency -- `sx1262_set_rf_frequency(p, freq_hz)`

Takes frequency directly in Hz (driver handles the
`RfFreq = freq_hz * 2^25 / Fxtal` conversion, `Fxtal = 32MHz`). E.g.
`sx1262_set_rf_frequency(p, 434000000UL)` for 434.000MHz.

### Image calibration -- `sx1262_calibrate_image(p, freq1, freq2)`

Must be called after `sx1262_set_rf_frequency()` for whichever ISM band
covers the chosen frequency (datasheet Table 9-2):

| Band [MHz] | freq1 | freq2 | Constant names |
|---|---|---|---|
| 430-440 | 0x6B | 0x6F | `SX1262_CAL_IMG_430_440_FREQ1/FREQ2` |
| 470-510 | 0x75 | 0x81 | -- |
| 779-787 | 0xC1 | 0xC5 | -- |
| 863-870 | 0xD7 | 0xDB | -- |

## LoRa modulation -- `sx1262_set_modulation_params_lora(p, sf, bw, cr, ldro)`

### Bandwidth (`SX1262_LORA_BW_*`, datasheet Table 6-2)

| Constant | BW |
|---|---|
| `SX1262_LORA_BW_7`   | 7.81 kHz |
| `SX1262_LORA_BW_10`  | 10.42 kHz |
| `SX1262_LORA_BW_15`  | 15.63 kHz |
| `SX1262_LORA_BW_20`  | 20.83 kHz |
| `SX1262_LORA_BW_31`  | 31.25 kHz |
| `SX1262_LORA_BW_41`  | 41.67 kHz |
| `SX1262_LORA_BW_62`  | 62.5 kHz |
| `SX1262_LORA_BW_125` | 125 kHz |
| `SX1262_LORA_BW_250` | 250 kHz |
| `SX1262_LORA_BW_500` | 500 kHz |

### Spreading factor: `SX1262_LORA_SF5` .. `SX1262_LORA_SF12` (values 0x05-0x0C)

### Coding rate: `SX1262_LORA_CR_4_5` (1), `_4_6` (2), `_4_7` (3), `_4_8` (4)

### LDRO (`SX1262_LORA_LDRO_OFF`/`_ON`)

Set ON when the symbol time `2^SF / BW >= 16.38ms` (typically SF11/BW125,
SF12/BW125, SF12/BW250).

## Time-on-air formula

```
T_sym_us  = 2^SF / BW_kHz * 1000
preamble_time_us = (n_preamble + 4.25) * T_sym_us
n_payload_symbols = 8 + max(ceil((8*PL - 4*SF + 28 + 16*CRC - 20*IH) / (4*(SF - 2*DE))) * (CR+4), 0)
payload_time_us = n_payload_symbols * T_sym_us
time_on_air_us = preamble_time_us + payload_time_us
```

Where `PL` = payload length (bytes), `CRC` = 1 if CRC enabled, `IH` = 1 if
implicit header (0 if explicit), `DE` = 1 if LDRO on, `CR` = coding rate
number from `SX1262_LORA_CR_4_5`=1 .. `_4_8`=4.

## Bench config history (for context)

The bench test config (`Core/Src/Tests/test_sx1262.c`,
`test_sx1262_config()`) has moved through:
- 434.000MHz -> 434.400MHz -> back to 434.000MHz (RSGB 70cm band plan:
  434.000MHz sits in the 433.800-434.250MHz "Digital communications &
  Experiments" segment)
- BW125 (kept; BW500 was tried and reverted -- doesn't fit any RSGB
  70cm band-plan segment near 434MHz)
- TX power: +17dBm -> +20dBm -> +14dBm -> +10dBm (current, ~10mW, to sit
  within the Ofcom IR2030/1/10 SRD class -- 10mW e.r.p., <=10% duty cycle
  -- pending confirmation of amateur-licence operating conditions for
  bench testing, see `sdd/README.md` "Ofcom IR2030 SRD Limits")
