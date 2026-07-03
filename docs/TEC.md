# Ionospheric TEC Monitoring — Future Goal

Using the GPS Staff permanent base at the garden benchmark as a single-station
ionospheric Total Electron Content (TEC) monitor, visualised in real time on a
Three.js globe.

---

## Concept

Once the permanent base is established at a surveyed benchmark it can simultaneously:

1. Stream RTCM corrections to rtk2go as a reference station (fixed-position mode)
2. Run as a rover against the same RTCM stream

Because the physical position is precisely known, any deviation in the rover
solution is not real movement — it is atmospheric and multipath error. The
ionospheric component of that error varies by satellite direction, so with 20–30
satellites in view across GPS, Galileo, GLONASS and BeiDou, you sample the
ionosphere in 20–30 different directions simultaneously.

During a solar-induced geomagnetic storm the ionospheric electron density rises,
becomes structured, and drifts toward the poles. Watching this in real time on a
globe — with coloured blobs at each satellite's ionospheric pierce point — is
essentially a personal space weather instrument.

---

## The raw data source — UBX-RXM-RAWX

The ZED-F9P outputs raw measurements via the **UBX-RXM-RAWX** message (section
1.5.1 of the ZED-F9P-05B datasheet, UBXDOC-963802114-12824). The message
follows RINEX 3 observation file conventions and is available at up to 10 Hz.

Each epoch contains one measurement block per tracked satellite per signal:

| Field | Type | Description |
|---|---|---|
| `prMes` | double (m) | Pseudorange measurement |
| `cpMes` | double (cycles) | Carrier phase measurement |
| `doMes` | float (Hz) | Doppler measurement |
| `gnssId` | u8 | Constellation: 0=GPS, 2=Galileo, 3=BeiDou, 6=GLONASS |
| `svId` | u8 | Satellite vehicle number |
| `sigId` | u8 | Signal: L1C/A, L2 CL/CM, E1, E5b, B1I, B2I… |
| `cno` | u8 (dBHz) | Carrier-to-noise density — signal quality |
| `trkStat` | flags | Bit 0: pseudorange valid; bit 1: carrier phase valid |

Full protocol definition: u-blox Interface Description UBX-18010854.

The F9P tracks **two frequencies per satellite** — GPS L1+L2, Galileo E1+E5b,
GLONASS L1+L2, BeiDou B1+B2. That dual-frequency pair is the key to extracting
the ionospheric delay without knowing the satellite geometry.

---

## Extracting TEC from dual-frequency pseudoranges

The ionospheric delay is proportional to Total Electron Content (TEC) and
inversely proportional to the square of the signal frequency:

```
I_L1 [m] = 40.3 × 10¹⁶ × TEC / f1²
```

The **geometry-free combination** subtracts one frequency's pseudorange from the
other. All geometry (satellite position, receiver clock, troposphere) cancels,
leaving only the differential ionospheric delay:

```
P4 = PR_L2 − PR_L1 = I_L1 × (f1²/f2² − 1)
```

Rearranging to absolute TEC in TECU (1 TECU = 10¹⁶ electrons/m²):

```
TEC [TECU] = (PR_L2 − PR_L1) × f1² × f2² / (40.3 × (f2² − f1²))
```

For GPS (f1 = 1575.42 MHz, f2 = 1227.60 MHz): **1 TECU ≈ 0.16 m on L1**.

Typical mid-latitude TEC values:

| Condition | TEC (TECU) |
|---|---|
| Night, quiet | 2–5 |
| Day, quiet | 10–20 |
| Geomagnetic storm | 50–100+ |

### Carrier phase combination (higher precision)

The carrier phase geometry-free combination:

```
L4 = λ₁φ₁ − λ₂φ₂ = −I_L1 × (1 − f1²/f2²) + constant
```

The constant contains an unknown integer ambiguity per satellite pass. L4 cannot
give absolute TEC without resolving the ambiguity, but it tracks *changes* in TEC
at millimetre precision — ideal for watching rapid ionospheric disturbances during
a storm even before ambiguity resolution is implemented.

---

## Computing the ionospheric pierce point (IPP)

Each satellite's signal pierces the ionosphere at a single geographic point,
conventionally modelled as a thin shell at **350 km altitude**. Given the fixed
receiver position (φ_rx, λ_rx) and the satellite's elevation E and azimuth A
(available from UBX-NAV-SAT):

```
ψ     = π/2 − E − arcsin(Rₑ / (Rₑ + 350) × cos E)   # Earth central angle
φ_IPP = arcsin(sin φ_rx × cos ψ + cos φ_rx × sin ψ × cos A)
λ_IPP = λ_rx + arcsin(sin ψ × sin A / cos φ_IPP)
```

where Rₑ = 6371 km. The result (φ_IPP, λ_IPP) is the geographic coordinate
where the TEC measurement should be plotted on the globe.

---

## Multipath separation

Multipath from the garden environment will contaminate P4. It is site-specific
and repeats every **sidereal day (23 h 56 min)** because the satellite geometry
repeats on that period. After collecting several quiet-day baselines, the
multipath signature can be subtracted epoch-by-epoch, leaving a clean
ionospheric residual. This is the same technique used by professional
Continuously Operating Reference Stations (CORS) such as OS Net.

Practical approach:
1. Collect a week of quiet-day RAWX data
2. For each satellite/signal pair, build a sidereal-repeat template of P4
3. Subtract the template — residual is ionospheric variation

---

## Processing pipeline

```
ZED-F9P UART
  └── UBX-RXM-RAWX (10 Hz, all constellations)
  └── UBX-NAV-SAT  (1 Hz, elevation + azimuth per satellite)
        │
        ▼
  STM32F765 (or direct USB to PC)
        │
        ▼
  Per satellite, per epoch:
    P4  = PR_L2 − PR_L1
    TEC = P4 × f1² × f2² / (40.3 × (f2² − f1²))
    IPP = pierce_point(φ_rx, λ_rx, elev, azim)
        │
        ▼
  Three.js globe
    Blob at (φ_IPP, λ_IPP), size/colour = TEC magnitude
    Animated in real time, 1–10 Hz update
```

With 20–30 satellites visible simultaneously, you get 20–30 pierce points
scattered across a roughly 2000 km radius patch of ionosphere above the UK,
updated continuously.

---

## What to watch for

| Event | Observable signature |
|---|---|
| Solar flare (X-class) | Sudden TEC increase on the sunlit side, all satellites simultaneously |
| Geomagnetic storm | Structured blobs drifting equatorward, enhanced at high elevations |
| Travelling ionospheric disturbance (TID) | Wave-like pattern moving across pierce points over minutes |
| Quiet night | All blobs small and stable |
| Sunrise/sunset | Smooth TEC ramp up/down, visible as blobs brightening/fading |

These are the same phenomena that affect HF propagation (the bands you use under
the GM4AJK licence) and that degrade other people's RTK solutions during storms —
observed here from a garden instrument.

---

## Prerequisites before starting this work

- [ ] Permanent base PCB installed at surveyed garden benchmark
- [ ] RTCM stream running to rtk2go (Mode 6 — Fixed Base)
- [ ] UBX-RXM-RAWX and UBX-NAV-SAT enabled on the F9P UART
- [ ] RAWX logging to SD card (or direct USB stream to PC) implemented in firmware
- [ ] At least one week of quiet-day baseline data collected for multipath template
- [ ] Three.js satellite visualiser adapted to accept real-time TEC + IPP data
