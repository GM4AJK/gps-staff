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
| 0xAD00 | *reserved — next service block* |

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

## Planned Services (not yet assigned)

| Proposed service | Peripheral | Central | Connection lifetime | Notes |
|-----------------|------------|---------|---------------------|-------|
| Rover status + Base battery relay | ESP32 Rover | Handheld | Permanent (always in proximity) | Position fix, RTCM lock, rover battery; Base battery % piggybacked after arriving over GFSK from Base → Rover STM32 → Rover ESP32 → BLE → HH. RTCM flowing = Base operational; battery is the only health data RTCM doesn't carry |
| Base Config channel | ESP32 Base | Handheld | Temporary — HH connects on entering Base Config UI, drops on exit | WiFi provisioning (0xAC00 already exists), future config commands; user must be in proximity by definition |

### Connection lifetime notes

- **HH ↔ Rover**: permanent connection while HH is powered. Rover is always in the user's hand/pocket.
- **HH ↔ Base**: temporary, UI-driven. `ble_base_client.c` must suppress auto-rescan outside of Base Config screen — reconnect is always initiated by the user entering that UI section, never automatic.
- **Base battery in the field**: Base battery % flows Base STM32 → GFSK → Rover STM32 → Rover ESP32 → BLE → HH. RTCM flowing already confirms Base is alive; battery % is the only additional data needed.
