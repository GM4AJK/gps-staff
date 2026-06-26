# 00001 — WiFi Provisioning

| Field | Value |
|-------|-------|
| Status | Phase 1.1 implemented |
| Phase 1 PR | #206 (Phase 1.1 — unprovisioned WiFi scan + BLE advertise) |
| Phase 2 PR | — |
| Phase 3 PR | — |

---

## Overview

The Base unit scans for nearby WiFi networks and allows the user to select one and enter a password via the Handheld. Once credentials are accepted, the Base connects to WiFi and stores the credentials for automatic reconnection on future boots. This enables the Base to stream RTCM correction data to an internet-based NTRIP caster.

The Rover is not involved in this flow.

---

## Phase 1 — Base advertises state over BLE

**Firmware:** `firmware/esp32-base`

The Base has two distinct boot states, and advertises different data depending on which it is in. The BLE advertisement updates every 3 seconds in both states.

### 1.1 — Unprovisioned (no stored credentials)

- Base scans for nearby WiFi APs using `espressif/network_provisioning` managed component
- Advertises the AP list over BLE (SSID, RSSI, auth type per entry)
- If no APs are found, advertises an empty list — the HH must handle this and inform the user that no WiFi networks are available
- Continues scanning and updating until credentials are received
- Scan cycle time is hardware-dependent (typically 2–4 seconds across all channels); advertisement must not update more frequently than once every 3 seconds

### 1.2 — Provisioned (stored credentials present)

- Base connects to the stored WiFi network at boot
- Advertises connection state over BLE every 3 seconds instead of an AP scan list
- Advertisement includes: connected SSID, current RSSI, assigned IP address
- No AP scanning occurs while connected

**Handheld behaviour (Phase 2 detail):**
- If advertisement contains an AP list → show WiFi setup screen
- If advertisement contains connection state → show "Base: Connected to [SSID]" status; no setup screen

**Acceptance criteria:**
- Unprovisioned: Handheld receives AP list updates no more frequently than once every 3 seconds
- Provisioned: Handheld receives connection state (SSID + RSSI + IP) every 3 seconds
- Handheld can distinguish the two advertisement types and branch accordingly

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
