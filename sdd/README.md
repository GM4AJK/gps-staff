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

| Document | Contents |
|---|---|
| [features/00001-wifi-provisioning.md](features/00001-wifi-provisioning.md) | WiFi provisioning — all phases bench-verified |
| [features/00002-HomeScreen.html](features/00002-HomeScreen.html) | Home screen spec (1024×600 landscape) |
| [features/00003-BaseConfig-0.html](features/00003-BaseConfig-0.html) | Base Config — State 0: top-level mode selector (Survey-in / RTCM Streaming / Satellite View / config strip) |
| [features/00003-BaseConfig-1.html](features/00003-BaseConfig-1.html) | Base Config — State 1: normal view, network table |
| [features/00003-BaseConfig-2.html](features/00003-BaseConfig-2.html) | Base Config — State 2: password entry |
| [features/00003-BaseConfig-2-kbd.html](features/00003-BaseConfig-2-kbd.html) | Base Config — State 2 with virtual keyboard |
| [features/00003-BaseConfig-3.html](features/00003-BaseConfig-3.html) | Base Config — State 3: connecting (spinner) |
| [features/00003-BaseConfig-4.html](features/00003-BaseConfig-4.html) | Base Config — State 4: connection failed |
| [features/00003-BaseConfig-SurveyIn.html](features/00003-BaseConfig-SurveyIn.html) | Base Config — Survey-in (progress ring, condition chips, live stats, cancel; 3 states) |
| [features/00003-BaseConfig-Streaming.html](features/00003-BaseConfig-Streaming.html) | Base Config — RTCM Data (broadcast stats, RTCM message activity, rover connection + LoRa link quality) |
| [features/00003-BaseConfig-FixedPos.html](features/00003-BaseConfig-FixedPos.html) | Base Config — Fixed Position entry (monument/benchmark, OSGB36/WGS84) |
| [features/00004-BaseSatelliteView-1.html](features/00004-BaseSatelliteView-1.html) | Base Satellite View — sky plot (az/el, GPS+GLONASS), CN0 bar chart, fix status |
| [features/00005-AboutScreen-1.html](features/00005-AboutScreen-1.html) | About screen — product identity, firmware/hardware versions, the Wizard |
