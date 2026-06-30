# GPS Staff — Screen Implementation Index

Tracks implementation status per spec file. Updated as each screen is built.

**Status tags:**
- `[ ]` Not started
- `[~]` Partial — some items done, see detail
- `[x]` Complete — all items implemented, no fake data remaining
- `[F]` Fake data — implemented but contains placeholder values, needs revisit

---

## 00002 — Home Screen

**Status:** `[~]` Partial — layout complete, all live data is `[F]` placeholder

### Implemented
- [x] Dark theme layout (flex column, 1024×600)
- [x] Status bar (28px, bg_secondary)
- [x] Hero section — GNSS icon (arc+line primitives), "GPS Staff" title, tagline
- [x] Base card — styled with left accent border, bullets, "Configure Base" button
- [x] Rover card — dimmed/disabled, "Coming soon" badge, non-clickable
- [x] BLE connection dot — grey (never connected) placeholder
- [x] Version strip (bottom, "GPS STAFF · PCB v1.0 · fw 0.1.0")
- [x] Touch input wired to LVGL pointer input device (click events work)

### Fake data `[F]` — revisit when hardware live
- `[F]` Base battery % — placeholder "Base: --%" until BLE beacon live
- `[F]` Rover battery % — placeholder "Rover: --%" until BLE beacon live
- `[F]` GPS UTC date/time — "----/--/-- --:--:--" until Rover F9P + BLE live
- `[F]` Tablet battery % — placeholder "--%", needs ESP32 ADC read
- `[F]` BLE dot colour — grey; update to green/amber on BLE connect/disconnect event
- `[F]` "Configure Base" navigation — logs intent, no target screen yet

### Not yet implemented
- [ ] Operating Mode chip (navigates to 00006)
- [ ] Status bar battery bar-graph (`▐███░▌`) — needs RobotoMono or custom font with block chars

---

## 00003-0 — Base Config: Mode Select

**Status:** `[ ]` Not started

### To implement
- [ ] Tab strip: Survey-in / RTCM Data / Satellite View / WiFi
- [ ] Each tab navigates to correct sub-screen
- [ ] Config strip (mode, RF, NTRIP status chips) — `[F]` fake until BLE live

---

## 00003-1 — Base Config: WiFi Networks

**Status:** `[ ]` Not started

### To implement
- [ ] Left panel: current config + remembered slot count — `[F]` fake until BLE live
- [ ] Right panel: Remembered Networks section (Connected / In Range / Not in Range rows)
- [ ] Right panel: Other Networks section + Scan button
- [ ] Connect (remembered — no password prompt)
- [ ] Connect (other — navigate to 00003-2 password entry)
- [ ] Forget button → removes NVS entry
- [ ] Disconnect button

### Fake data / revisit
- `[F]` All WiFi data via BLE — blocked until BLE + Base ESP32 WiFi scan forwarding live

---

## 00003-2 — Base Config: Password Entry

**Status:** `[ ]` Not started

### To implement
- [ ] Password field + virtual keyboard
- [ ] Connect button → navigate to 00003-3

---

## 00003-2-kbd — Base Config: Password Entry + Keyboard

**Status:** `[ ]` Not started

### To implement
- [ ] Slide-in virtual keyboard state
- [ ] Preview strip showing masked password

---

## 00003-3 — Base Config: Connecting

**Status:** `[ ]` Not started

### To implement
- [ ] Spinner state while Base attempts WiFi association

---

## 00003-4 — Base Config: Connection Failed

**Status:** `[ ]` Not started

### To implement
- [ ] Error message, Retry button

---

## 00003-SurveyIn-A — Base Config: Survey-in Ready

**Status:** `[ ]` Not started

### To implement
- [ ] Editable duration and accuracy fields
- [ ] Numeric virtual keyboard (00003-SurveyIn-A-kbd)
- [ ] Start Survey-in button → navigate to 00003-SurveyIn

### Fake data / revisit
- `[F]` F9P fix status — blocked until F9P hardware live

---

## 00003-SurveyIn-A-kbd — Base Config: Survey-in Numeric Keyboard

**Status:** `[ ]` Not started

---

## 00003-SurveyIn — Base Config: Survey-in In Progress

**Status:** `[ ]` Not started

### To implement
- [ ] Progress ring, live stats, condition chips
- [ ] Cancel button

### Fake data / revisit
- `[F]` All stats from F9P — blocked until F9P hardware live

---

## 00003-SurveyIn-C — Base Config: Survey-in Complete

**Status:** `[ ]` Not started

### To implement
- [ ] Full green ring, Begin RTCM Streaming button

---

## 00003-Streaming — Base Config: RTCM Data

**Status:** `[ ]` Not started

### To implement
- [ ] Broadcast stats, RTCM message activity
- [ ] Rover connection status, LoRa/GFSK link quality

### Fake data / revisit
- `[F]` All stats — blocked until SX1262 + F9P + BLE live

---

## 00003-FixedPos — Base Config: Fixed Position Entry

**Status:** `[ ]` Not started

### To implement
- [ ] OSGB36 / WGS84 coordinate entry fields
- [ ] Virtual keyboard for numeric entry
- [ ] Save to NVS

---

## 00004-1 — Base Satellite View

**Status:** `[ ]` Not started

### To implement
- [ ] Sky plot (az/el, GPS+GLONASS)
- [ ] CN0 bar chart
- [ ] Fix status

### Fake data / revisit
- `[F]` All data from F9P — blocked until F9P hardware live

---

## 00005-1 — About Screen

**Status:** `[ ]` Not started

### To implement
- [ ] Product identity, firmware/hardware versions
- [ ] The Wizard
- [ ] **"Transfer Files" button** → navigate to 00013 (USB Storage)

### Fake data / revisit
- `[F]` Version strings — replace with build-time constants

---

## 00006 — Operating Mode Selector

**Status:** `[ ]` Not started

### To implement
- [ ] Mode 1 — Local RF Link (selectable, active)
- [ ] Modes 2–6 — greyed out, non-selectable (not yet implemented)
- [ ] Confirm button → navigates to relevant config screen
- [ ] NVS save on Confirm

### TODO (future modes, unlock when implemented)
- [ ] Mode 2 — Rover NTRIP Direct
- [ ] Mode 3 — Base via Relay Caster
- [ ] Mode 4 — Dual NTRIP Independent
- [ ] Mode 5 — Direct TCP
- [ ] Mode 6 — Fixed Base Station

---

## 00007-1 — Rover Config: Mode 1 Local RF

**Status:** `[ ]` Not started

### To implement
- [ ] Pole height entry field + numeric keyboard
- [ ] RF link status (RSSI / SNR / frame count)
- [ ] F9P fix type indicator

### Fake data / revisit
- `[F]` RF link stats — blocked until SX1262 + BLE live
- `[F]` F9P fix — blocked until F9P hardware live

---

## 00008 — Mode 5: Remote Listener

**Status:** `[ ]` Not started

### To implement
- [ ] Listen port config field
- [ ] Router port-forward reminder text
- [ ] TCP connection status (Idle / Connected)
- [ ] Client IP, bytes sent, timer

### Unlock when
- Mode 5 enabled in 00006

---

## 00009 — Mode 5: Mobile Connector

**Status:** `[ ]` Not started

### To implement
- [ ] Remote hostname + port fields
- [ ] Connect / Cancel / Disconnect button state machine
- [ ] TCP stream stats

### Unlock when
- Mode 5 enabled in 00006

---

## 00010 — Start Survey Session

**Status:** `[ ]` Not started

### To implement
- [ ] Filename field (auto-suggested from NVS counter)
- [ ] Title field (required)
- [ ] Description field (optional)
- [ ] SD card status (present / absent) — Tablet SD direct read
- [ ] Start Session: create file, write header (incl. CSF from first fix), increment NVS counter
- [ ] Start button disabled when SD absent or title empty

### Fake data / revisit
- `[F]` CSF in log header — computed from first RTK Fix; will be `0.000000` or omitted until F9P live
- `[F]` GPS timestamp in header — `--` until F9P UTC live

---

## 00011 — Rover Working Screen

**Status:** `[ ]` Not started

### To implement
- [ ] Live WGS84 lat/lon/height (7dp / 3dp) from BLE
- [ ] Fix quality block (No Fix / Float / RTK Fix) with colour coding
- [ ] hAcc / vAcc / satellite count
- [ ] RF corrections strip (RSSI / SNR / fps / last frame)
- [ ] Session header (title, elapsed timer, point counter)
- [ ] Captured points list (scrollable, 7 columns)
- [ ] Tap row to rename label
- [ ] Capture button: grab next UBX-NAV-PVT epoch, append row, write CSV record

### Fake data / revisit
- `[F]` All position + fix data — blocked until F9P + BLE live
- `[F]` RF stats — blocked until SX1262 + BLE live

---

## 00013 — USB Storage Active

**Status:** `[ ]` Not started

### To implement
- [ ] TinyUSB CDC → MSC switch on entry
- [ ] SD card stats read before FatFS unmount (sessions, size, last file)
- [ ] Connected state (amber)
- [ ] Ejected state (green) + Done button
- [ ] "Disconnect anyway…" confirmation dialog
- [ ] VBUS loss auto-exit (FatFS remount, return to prior screen)
- [ ] MSC → CDC switch on exit

---

*Last updated: 2026-06-30 — Home screen impl (PR #266)*
