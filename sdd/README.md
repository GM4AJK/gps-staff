# Software and System Design

## Architecture

System-level design: unit roles, technology decisions, data flow, BLE protocols.

| Document | Contents |
|---|---|
| [architecture/SYSTEM.md](architecture/SYSTEM.md) | System overview — unit roles, technology decisions, data flow, RTK concepts |
| [architecture/BASE.md](architecture/BASE.md) | Base unit — implemented capabilities and pending work |
| [architecture/ROVER.md](architecture/ROVER.md) | Rover unit — implemented capabilities and pending work |
| [architecture/HANDHELD.md](architecture/HANDHELD.md) | Handheld/tablet — implemented capabilities and pending work |
| [architecture/GATT.md](architecture/GATT.md) | BLE GATT service and characteristic registry |

## Features

Numbered feature specifications and UI screen specs.

> **When adding a new spec file:** also add it to the `screens` array in
> [`features/00000-Overview.html`](features/00000-Overview.html) so it appears
> in the spec navigator.
>
> **When implementing a screen:** update
> [`features/completed-index.md`](features/completed-index.md) — tick off items
> as they are built and note anything implemented with fake/placeholder data so
> it gets revisited when the real hardware is live.

| Document | Contents |
|---|---|
| [features/00001-wifi-provisioning.md](features/00001-wifi-provisioning.md) | WiFi provisioning — all phases bench-verified |
| [features/00006-OperatingMode.html](features/00006-OperatingMode.html) | Operating Mode selector — 6 modes: Local RF, Rover NTRIP, Base via relay caster, Dual NTRIP, Direct TCP, Fixed Base Station; compact cards with no inline sub-selectors (sub-options live on each mode's config screen) |
| [features/00002-HomeScreen.html](features/00002-HomeScreen.html) | Home screen spec (1024×600 landscape) |
| [features/00003-BaseConfig-0.html](features/00003-BaseConfig-0.html) | Base Config — State 0: top-level mode selector (Survey-in / RTCM Streaming / Satellite View / config strip) |
| [features/00003-BaseConfig-1.html](features/00003-BaseConfig-1.html) | Base Config — State 1: normal view, network table |
| [features/00003-BaseConfig-2.html](features/00003-BaseConfig-2.html) | Base Config — State 2: password entry |
| [features/00003-BaseConfig-2-kbd.html](features/00003-BaseConfig-2-kbd.html) | Base Config — State 2 with virtual keyboard |
| [features/00003-BaseConfig-3.html](features/00003-BaseConfig-3.html) | Base Config — State 3: connecting (spinner) |
| [features/00003-BaseConfig-4.html](features/00003-BaseConfig-4.html) | Base Config — State 4: connection failed |
| [features/00003-BaseConfig-SurveyIn-A.html](features/00003-BaseConfig-SurveyIn-A.html) | Base Config — Survey-in State A: Ready (editable settings, Start button) |
| [features/00003-BaseConfig-SurveyIn-A-kbd.html](features/00003-BaseConfig-SurveyIn-A-kbd.html) | Base Config — Survey-in State A: Numeric keyboard (duration in minutes up to 5 digits; accuracy in metres; tab toggle between both fields) |
| [features/00003-BaseConfig-SurveyIn.html](features/00003-BaseConfig-SurveyIn.html) | Base Config — Survey-in State B: In Progress (progress ring, condition chips, live stats, cancel) |
| [features/00003-BaseConfig-SurveyIn-C.html](features/00003-BaseConfig-SurveyIn-C.html) | Base Config — Survey-in State C: Complete (full green ring, Begin RTCM Streaming button) |
| [features/00003-BaseConfig-Streaming.html](features/00003-BaseConfig-Streaming.html) | Base Config — RTCM Data (broadcast stats, RTCM message activity, rover connection + LoRa link quality) |
| [features/00003-BaseConfig-FixedPos.html](features/00003-BaseConfig-FixedPos.html) | Base Config — Fixed Position entry (monument/benchmark, OSGB36/WGS84) |
| [features/00004-BaseSatelliteView-1.html](features/00004-BaseSatelliteView-1.html) | Base Satellite View — sky plot (az/el, GPS+GLONASS), CN0 bar chart, fix status |
| [features/00005-AboutScreen-1.html](features/00005-AboutScreen-1.html) | About screen — product identity, firmware/hardware versions, the Wizard |
| [features/00007-RoverConfig-Mode1.html](features/00007-RoverConfig-Mode1.html) | Rover Config (Mode 1 — Local RF Link) — pole height entry, live RF link status (RSSI/SNR/frame count), F9P fix type; two dev states: Waiting / Receiving |
| [features/00008-Mode5-RemoteListener.html](features/00008-Mode5-RemoteListener.html) | Mode 5 Direct TCP — Remote Listener (whichever device stays on home WiFi): listen port config, router port-forward reminder, TCP connection status; two dev states: Idle / Connected |
| [features/00009-Mode5-MobileConnector.html](features/00009-Mode5-MobileConnector.html) | Mode 5 Direct TCP — Mobile Connector (whichever device is in the field on hotspot): remote hostname + port entry, Connect/Disconnect button, TCP stream stats; three dev states: Disconnected / Connecting / Connected |
| [features/00013-UsbStorageActive.html](features/00013-UsbStorageActive.html) | Tablet — USB Storage Active: MSC holding screen; SD card info (sessions/size/last file) read before FatFS unmount; amber Connected / green Ejected states; "Disconnect anyway…" confirmation guard; documents CDC↔MSC switching and FatFS mutual exclusion |
| [features/00010-StartSurveySession.html](features/00010-StartSurveySession.html) | Rover — Start Survey Session: filename (auto-suggested, FAT-safe), title (required), description (optional); log written to Tablet SD card directly (ESP32-S3 owns the SD slot — no UART extension needed) |
| [features/00011-RoverWorking.html](features/00011-RoverWorking.html) | Rover — Working Screen: live WGS84 position (7 dp lat/lon, 3 dp height MSL), fix quality block (No Fix / RTK Float / RTK Fix), RF corrections status, scrollable captured-points list (ID/label/lat/lon/hAcc), Capture button grabs next F9P epoch and appends row immediately; three dev states |
