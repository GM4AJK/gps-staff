# Firmware Data

A catalog of binary/data files used by firmware for testing or simulation.

| File | Description |
|---|---|
| [CambridgeSensoriisSample.bin](CambridgeSensoriisSample.bin) | Recorded RTCM3 stream (2728 bytes, 39 frames: 1005/1074/1084 cycle, GPS+GLONASS MSM4), captured from the `CambridgeSensoriis` RTK2GO NTRIP mount. Intended as a canned playback payload for a Nucleo-based "fake F9P" base, to test the base->LoRa->rover transport pipeline without real GNSS hardware |
