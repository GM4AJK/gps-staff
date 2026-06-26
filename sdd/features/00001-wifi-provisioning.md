# 00001 — WiFi Provisioning

| Field | Value |
|-------|-------|
| Status | Draft |
| Phase 1 PR | — |
| Phase 2 PR | — |
| Phase 3 PR | — |

---

## Overview

The Base unit scans for nearby WiFi networks and allows the user to select one and enter a password via the Handheld. Once credentials are accepted, the Base connects to WiFi and stores the credentials for automatic reconnection on future boots. This enables the Base to stream RTCM correction data to an internet-based NTRIP caster.

The Rover is not involved in this flow.

---

## Phase 1 — Base scans and advertises over BLE

**Firmware:** `firmware/esp32-base`

- Base ESP32 scans for nearby WiFi APs using `espressif/network_provisioning` managed component
- Advertises the AP list over BLE so the Handheld can retrieve it
- Continues scanning and updating the list until credentials are received
- Scan cycle time is hardware-dependent (typically 2–4 seconds across all channels); the BLE advertisement must not update more frequently than once every 3 seconds regardless of scan duration

**Acceptance criteria:**
- Handheld can connect to Base over BLE and retrieve a list of visible SSIDs with RSSI and auth type
- List updates no more frequently than once every 3 seconds

---

## Phase 2 — Handheld presents AP list and sends credentials

**Firmware:** `firmware/esp32-handheld`

- Handheld connects to Base over BLE and displays the received AP list
- User selects an SSID from the list
- If the network requires a password, a text input is presented for entry
- Handheld sends selected SSID + password back to Base over BLE

**Acceptance criteria:**
- AP list visible and scrollable on Handheld screen
- User can select a network and enter a password via touchscreen keyboard
- Credentials transmitted to Base and acknowledged

---

## Phase 3 — Base connects to WiFi and stores credentials

**Firmware:** `firmware/esp32-base`

- Base receives SSID + password from Handheld
- Stops scanning and attempts WiFi connection

### 3.1 — Successful connection
- Stores credentials in NVS
- Connects to that WiFi network automatically on all future boots
- Notifies Handheld of success

### 3.2 — Failed connection
- Returns to scanning and advertising
- Includes failure reason in advertised data (wrong password, network unreachable, etc.)
- Handheld displays failure reason to user

### 3.3 — User action on failure
- TBD — options include: re-enter password, select a different network, cancel

**Acceptance criteria:**
- Successful connection persists across power cycles without re-provisioning
- Failed connection returns cleanly to provisioning mode with reason visible on Handheld

---

## Notes

- `espressif/network_provisioning` (v^1.0.5) is the IDF 6.2 replacement for the removed `wifi_provisioning` built-in component. Add to `firmware/esp32-base/main/idf_component.yml`.
- NTRIP streaming (using the WiFi connection established here) is out of scope for this spec — see future feature spec.
- The BLE link used for provisioning is the same NimBLE stack already in `firmware/esp32-base` (PR #187). Provisioning reuses this transport rather than adding a second BLE stack.
