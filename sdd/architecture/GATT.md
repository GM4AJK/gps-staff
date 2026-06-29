# GATT Service Registry

All BLE services and characteristics used across the GPS Staff system.
Assign new UUIDs here first to avoid collisions.

---

## UUID Registry

| UUID   | Assigned to |
|--------|-------------|
| 0xAB00 | ~~RTCM Bridge Service~~ — retired; RTCM flows over GFSK (SX1262), not BLE |
| 0xAB01 | ~~RTCM Bridge — data characteristic~~ — retired |
| 0xAC00 | WiFi Provisioning Service |
| 0xAC01 | WiFi Provisioning — AP list characteristic |
| 0xAC02 | WiFi Provisioning — credential write characteristic |
| 0xAC03 | WiFi Provisioning — connection result characteristic |
| 0xAD00 | Base Telemetry Service |
| 0xAD01 | Base Telemetry — BASE_OPERATING_MODE characteristic |
| 0xAD02 | Base Telemetry — SURVEY_IN_STATUS characteristic |
| 0xAD03 | Base Telemetry — SURVEY_IN_COMMAND characteristic |
| 0xAD04 | Base Telemetry — SET_SURVEY_PARAMS characteristic |
| 0xAD05 | Base Telemetry — GNSS_SAT_DATA characteristic |
| 0xAD06 | Base Telemetry — GNSS_FIX_STATUS characteristic |
| 0xAD07 | Base Telemetry — RTCM_STREAM_STATUS characteristic |
| 0xAD08 | ~~ROVER_LINK_STATUS~~ — removed; Base is a pure broadcaster, rover status flows Base←→Handheld directly |
| 0xAD09 | Base Telemetry — RTCM_STREAM_COMMAND characteristic |
| 0xAD0A | Base Telemetry — GFSK_RADIO_CONFIG characteristic |
| 0xAE00 | *reserved — next service block* |

---

## Service Definitions

### 0xAC00 — WiFi Provisioning

**Purpose:** Base advertises visible WiFi APs to the Handheld; Handheld returns selected SSID + password.

| Role | Device | Firmware |
|------|--------|----------|
| Peripheral (GATT server) | ESP32-S3 Zero — base role | `firmware/esp32-base` (`wifi_prov.c`) |
| Central (GATT client) | ESP32-S3 4.3" Handheld | `firmware/esp32-handheld` (`ble_base_client.c`) |

> Refactor note: changes to this service require updates to both `wifi_prov.c` (peripheral) on the base and `ble_base_client.c` + `wifi_provision.c` (central + UI) on the handheld.

#### 0xAC01 — AP List (Base → Handheld)

Properties: **NOTIFY**

Sent every 3 seconds. An AP count of 0 means no networks were found.

```
Byte 0:     AP count (uint8_t)
Per AP (repeated count times):
  Byte:     ssid_len (uint8_t, 0–32)
  N bytes:  SSID (not null-terminated)
  Byte:     rssi (int8_t cast to uint8_t)
  Byte:     auth_mode (uint8_t, see Auth Mode Values below)
```

#### 0xAC02 — Credentials (Handheld → Base)

Properties: **WRITE** (confirmed)

Sent once when the user taps Connect. Empty password string = open network.

```
Byte 0:     ssid_len (uint8_t, 0–32)
N bytes:    SSID
Byte:       pwd_len (uint8_t, 0–64)
M bytes:    password
```

#### 0xAC03 — Connection Result (Base → Handheld)

Properties: **NOTIFY**

Sent once when a connection attempt completes (success or failure). Re-sent as a 3-second heartbeat while Base remains connected (allows Handheld to detect already-provisioned Base on connect).

```
Byte 0:     status
              0 = connected (success)
              1 = wrong password
              2 = network not found
              3 = connection timed out
              4 = other error
Bytes 1–33: SSID null-terminated (32 chars max + null; zero-filled if shorter)
Byte 34:    RSSI of connected AP (int8_t as uint8_t; 0 if status ≠ 0)
Bytes 35–38: IP address (uint32_t little-endian; 0.0.0.0 if status ≠ 0)
```

#### Auth Mode Values

Mirrors `wifi_auth_mode_t` from ESP-IDF. Defined as `PROV_AUTH_*` in `ble_base_client.h`.

| Value | Meaning |
|-------|---------|
| 0 | Open (no password) |
| 1 | WEP |
| 2 | WPA |
| 3 | WPA2 |
| 4 | WPA/WPA2 mixed |
| 5 | Enterprise |
| 6 | WPA3 |
| 7 | WPA2/WPA3 mixed |

---

---

## 0xAD00 — Base Telemetry Service

**Purpose:** Exposes live Base operating state, GNSS telemetry, and survey-in
control to the Handheld. All readable characteristics must be polled on every
fresh BLE connection (see Reconnect / State Discovery below).

| Role | Device | Firmware |
|------|--------|----------|
| Peripheral (GATT server) | ESP32-S3 Zero — base role | `firmware/esp32-base` |
| Central (GATT client) | ESP32-S3 Tablet / Handheld | `firmware/esp32-tablet` / `esp32-handheld` |

### State ownership

**All application state lives in the ESP32-S3-Zero — not the STM32.**

The STM32F765 is a smart pipe: it parses UBX frames from the F9P and forwards
condensed binary messages to the ESP32 over UART, and executes commands the ESP32
sends back (start/cancel SVIN, write UBX-CFG-TMODE3, enable LoRa TX). It has no
knowledge of BLE, GATT, or operating mode.

The ESP32 synthesises `BASE_OPERATING_MODE` from two inputs:
  1. F9P telemetry forwarded by the STM32 (SVIN active/valid, fix type, etc.)
  2. Its own command state (e.g. "I sent Start SVIN → mode = 0x01").

State is held in ESP32 RAM only. The STM32 and ESP32 share the same power supply;
if the Base is switched off, all state is lost and the system performs a clean start
on next power-on. No NVS persistence is required or expected.

---

### Reconnect / State Discovery

The HH ↔ Base BLE connection is **temporary and UI-driven** — it is established
when the user opens Base Config and dropped on exit. The tablet may also have been
powered off entirely. Either way, the tablet has no knowledge of what the Base has
been doing since the connection was last open.

**On every fresh connection, the tablet must:**
1. Read `BASE_OPERATING_MODE` (0xAD01) — single byte, immediate.
2. Navigate to the appropriate screen based on the value (see table below).
3. Subscribe to notifications for whichever characteristics the current screen needs.
4. Read those characteristics immediately after subscribing to get current values
   (do not wait for the first notify; it may take up to 1 s).

This sequence handles tablet power cycles, normal UI re-entry, and long gaps between
Base Config visits identically — no special "reconnect" code path is needed.

**Do NOT** infer Base state by combining multiple characteristics and applying logic.
`BASE_OPERATING_MODE` is the single source of truth; the Base STM32/ESP32 owns it.

---

### 0xAD01 — BASE_OPERATING_MODE

Properties: **READ + NOTIFY** (notify on mode transition)

Single byte. Owned and updated by the Base ESP32 application layer. The ESP32
synthesises the value from F9P telemetry forwarded by the STM32 (SVIN active/valid,
fix type) combined with its own command state (e.g. "I sent Start SVIN"). The STM32
has no knowledge of this characteristic.

```
0x00  Idle            No active mode, no valid position held.
                      → BaseConfig-0: all cards enabled, position banner = None
0x01  Survey-in active  F9P SVIN running.
                      → Navigate to BaseConfig-SurveyIn.html, State B (in progress)
                      → BaseConfig-0: RTCM Data card disabled
0x02  Survey-in complete  Valid surveyed position held (SVIN valid=1).
                      → Navigate to BaseConfig-SurveyIn.html, State C (complete)
                      → BaseConfig-0: position banner = Survey-in, RTCM Data enabled
0x03  Fixed position set  Monument/benchmark coordinates stored.
                      → BaseConfig-0: position banner = Monument, RTCM Data enabled
0x04  RTCM streaming active  Base is broadcasting corrections over LoRa.
                      → BaseConfig-0: RTCM Data card highlighted / active
0xFF  Initialising    STM32 not yet reported mode (ESP32 just booted).
                      → Show spinner; retry read after 2 s
```

---

### 0xAD02 — SURVEY_IN_STATUS

Properties: **READ + NOTIFY** (1 Hz while active; last value readable at any time)

```
Byte  0     active   uint8   1 = survey-in currently running on F9P
Byte  1     valid    uint8   1 = survey-in complete (both conditions met)
Bytes 2–5   dur      uint32  elapsed duration, seconds (little-endian)
Bytes 6–9   obs      uint32  position observations accumulated
Bytes 10–13 meanAcc  uint32  current mean position accuracy, mm
Bytes 14–15 minDur   uint16  configured target duration, seconds
Bytes 16–19 minAcc   uint32  configured target accuracy, mm
```

**minDur and minAcc must be included** so a freshly-connected tablet can display
the correct targets without a separate parameter read. The STM32 echoes the values
it last wrote to the F9P via UBX-CFG-VALSET.

Source: UBX-NAV-SVIN from Base F9P, parsed by Base STM32, forwarded over UART to ESP32.

---

### 0xAD03 — SURVEY_IN_COMMAND

Properties: **WRITE NO RESPONSE**

```
0x01  Start   Begin survey-in with parameters from SET_SURVEY_PARAMS (0xAD04).
              STM32 writes UBX-CFG-VALSET to F9P then enables TMODE3.
              Ignored if active=1 already.
0x02  Cancel  Stop survey-in. STM32 sends UBX-CFG-TMODE3 mode=0 to F9P.
              BASE_OPERATING_MODE transitions to 0x00 (Idle) or 0x02 if valid.
```

---

### 0xAD04 — SET_SURVEY_PARAMS

Properties: **WRITE NO RESPONSE** (send before SURVEY_IN_COMMAND 0x01)

```
Bytes 0–1   minDur   uint16  target duration, seconds (180–600 recommended)
Bytes 2–5   minAcc   uint32  target accuracy, mm (e.g. 3000 = 3.0 m)
```

---

### 0xAD05 — GNSS_SAT_DATA

Properties: **NOTIFY** (1 Hz; not readable — too large, always from latest notify)

One record per tracked satellite (up to 20). Each record is 7 bytes:

```
Byte 0   gnssId    uint8   0=GPS, 6=GLONASS
Byte 1   svId      uint8   satellite number within constellation
Byte 2   elev      uint8   elevation, degrees (0–90)
Bytes 3–4 azim     uint16  azimuth, degrees (0–359, little-endian)
Byte 5   cno       uint8   carrier-to-noise, dBHz (0–50 typical)
Byte 6   flags     uint8   bit 0 = usedInFix
```

Source: UBX-NAV-SAT from Base F9P. F9P must have NAV-SAT enabled at 1 Hz.

---

### 0xAD06 — GNSS_FIX_STATUS

Properties: **READ + NOTIFY** (1 Hz)

```
Byte  0     fixType   uint8   0=no fix, 3=3D, 4=GNSS+DR, 5=Time only
Byte  1     carrSoln  uint8   0=none, 1=RTK float, 2=RTK fixed
Byte  2     numSV     uint8   satellites used in fix
Bytes 3–4   pDOP      uint16  pDOP × 100 (little-endian)
Bytes 5–6   hDOP      uint16  hDOP × 100
Bytes 7–10  iTOW      uint32  GPS time of week, ms
```

RTK float = fixType≥3 + carrSoln=1. RTK fixed = fixType≥3 + carrSoln=2.
Source: UBX-NAV-PVT from Base F9P.

---

### 0xAD07 — RTCM_STREAM_STATUS

Properties: **READ + NOTIFY** (1 Hz while streaming; single READ returns current snapshot)

```
Byte  0     streaming   uint8   1 = actively broadcasting over LoRa; 0 = idle
Byte  1     msg_mask    uint8   bit 0 = RTCM 1005 seen in last 2 s
                                bit 1 = RTCM 1074 seen in last 2 s
                                bit 2 = RTCM 1084 seen in last 2 s
Bytes 2–3   byte_rate   uint16  bytes/sec forwarded to LoRa (5 s rolling average)
Byte  4     pkt_rate    uint8   RTCM packets/sec (5 s rolling average)
Byte  5     _pad        uint8   reserved, 0
Bytes 6–9   total_pkts  uint32  total packets forwarded since Base boot (little-endian)
Bytes 10–13 total_bytes uint32  total bytes forwarded since Base boot (little-endian)
```

Source: Base STM32 counters (RTCM framer / LoRa TX path), forwarded to ESP32 over UART.
Session counters reset on power cycle (not persisted).

---

### 0xAD08 — ~~ROVER_LINK_STATUS~~ (removed)

Removed: the Base is a pure GFSK broadcaster with no concept of connected rovers —
there may be zero, one, or many, or the receiver may be an NTRIP caster rather than
a rover. Rover status (fix quality, RSSI, battery) flows directly between the Rover
ESP32 and the Handheld over BLE; it does not route through the Base.

---

### 0xAD09 — RTCM_STREAM_COMMAND

Properties: **WRITE NO RESPONSE**

```
0x01  Stop streaming    BASE_OPERATING_MODE → 0x02 (position held, not streaming).
                        STM32 stops forwarding RTCM packets to LoRa TX.
0x02  Resume streaming  BASE_OPERATING_MODE → 0x04. Valid only if mode was 0x02.
                        Ignored if no valid position is held.
```

---

### 0xAD0A — GFSK_RADIO_CONFIG

Properties: **READ + WRITE** (no notify — config changes are infrequent)

Same 18-byte layout for both READ response and WRITE command.
On WRITE, STM32 reconfigures SX1262 immediately and persists to STM32 flash.
Tablet confirms the write by issuing a READ after ~200 ms and updating the display.

```
Bytes  0–3   freq_hz    uint32   carrier frequency, Hz (e.g. 434000000 = 434.000 MHz)
Byte   4     tx_power   int8     TX power, dBm. UI presents 6 fixed levels:
                                 −17 (min/test), 0 (1 mW, default), +10 (10 mW),
                                 +14 (25 mW), +17 (50 mW), +22 (158 mW, max batt drain).
                                 Wire value is raw dBm int8; SX1262 PA_CONFIG limits apply.
Bytes  5–8   bit_rate   uint32   GFSK bit rate, bps (e.g. 9600)
Bytes  9–12  sync_word  uint32   sync word, big-endian
                                 e.g. 0x00906F26 for the 3-byte word 0x906F26
                                 Base and Rover must share the same value.
Bytes 13–14  freq_dev   uint16   frequency deviation, Hz (READ only; STM32 calculates
                                 as bit_rate / 2 for BT=0.5 Gaussian, modulation index β=1.0;
                                 ignored on WRITE)
Bytes 15–18  rx_bw      uint32   SX1262 RX bandwidth, Hz (READ only; STM32 selects the
                                 nearest available step above 2×freq_dev + bit_rate;
                                 ignored on WRITE)
```

**GFSK collision note:** The F9P outputs RTCM on the GPS timepulse (~1 Hz, UTC-aligned).
Two nearby base stations therefore transmit simultaneously with GPS-clock precision,
causing repeated RF collisions regardless of sync word. Sync word prevents a rover
accepting packets from the wrong base; it does not prevent the RF collision itself.
Mitigation: assign different frequencies to co-located bases.
LoRa spread-spectrum would be more collision-tolerant; GFSK is used here for duty-cycle
efficiency (7–8× less airtime, critical under ETSI EN 300 220 10% limit on 433/434 MHz).

---

## Planned Services (not yet assigned)

| Proposed service | Peripheral | Central | Connection lifetime | Notes |
|-----------------|------------|---------|---------------------|-------|
| Rover status + Base battery relay | ESP32 Rover | Handheld | Permanent (always in proximity) | Position fix, RTCM lock, rover battery; Base battery % piggybacked after arriving over GFSK from Base → Rover STM32 → Rover ESP32 → BLE → HH. RTCM flowing = Base operational; battery is the only health data RTCM doesn't carry |
| Base Config channel | ESP32 Base | Handheld | Temporary — HH connects on entering Base Config UI, drops on exit | WiFi provisioning (0xAC00 already exists), future config commands; user must be in proximity by definition |

### Connection lifetime notes

- **HH ↔ Rover**: permanent connection while HH is powered. Rover is always in the user's hand/pocket.
- **HH ↔ Base**: temporary, UI-driven. `ble_base_client.c` must suppress auto-rescan outside of Base Config screen — reconnect is always initiated by the user entering that UI section, never automatic.
- **Base battery in the field**: Base battery % flows Base STM32 → GFSK → Rover STM32 → Rover ESP32 → BLE → HH. RTCM flowing already confirms Base is alive; battery % is the only additional data needed.
