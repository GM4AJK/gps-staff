# OSTN15/OSGM15: ETRS89 → OSGB36/ODN transform pipeline

This document describes how to generate the binary grid file used by the app
to convert ETRS89 coordinates to OSGB36 easting/northing and ODN height.

## Background

The ZED-F9P outputs coordinates in ETRS89 (effectively WGS84 at the sub-metre
level). Ordnance Survey Great Britain uses two coordinate systems:

- **OSGB36** — the National Grid (eastings/northings), based on the Airy 1830
  ellipsoid and Transverse Mercator projection
- **ODN** — Ordnance Datum Newlyn, the vertical datum for Great Britain

The OS publishes **OSTN15** (horizontal) and **OSGM15** (vertical) shift grids
that convert ETRS89 to OSGB36/ODN. They are the definitive transforms; no
analytical formula exists.

The transform is display-only. All measurements and logging stay in ETRS89.

## Prerequisites

- Python 3.8+ (`python3` on PATH)
- The **OSTN15_OSGM15-DevelopersPack.zip** from Ordnance Survey
  (free download from the OS website, ~40 MB zip)
- The repo scripts: `scripts/ostn15_convert.py`, `scripts/ostn15_verify.py`

## Step 1 — extract the developer pack

Unzip into `tmp/` (which is git-excluded):

```
mkdir -p tmp/ostn15_osgm15
unzip /path/to/OSTN15_OSGM15-DevelopersPack.zip -d tmp/ostn15_osgm15/
```

Confirm these files exist afterwards:

```
tmp/ostn15_osgm15/OSTN15_OSGM15_DataFile.txt          (~40 MB)
tmp/ostn15_osgm15/OSTN15_OSGM15_TestInput_ETRStoOSGB.txt
tmp/ostn15_osgm15/OSTN15_OSGM15_TestOutput_ETRStoOSGB.txt
```

## Step 2 — convert to compact binary

```
python3 scripts/ostn15_convert.py
```

This reads the 40 MB text file (~877,000 comma-separated records) and writes a
10 MB binary at `tmp/ostn15_osgm15/ostn15_osgm15.bin`.

### Binary format

```
Offset  Length  Type      Value
0       8       ASCII     magic b'OSTN1500'
8       4       uint32 LE num_cols = 701  (E 0 to 700 000 m, 1 km step)
12      4       uint32 LE num_rows = 1251 (N 0 to 1 250 000 m, 1 km step)
16      876 951 × 12
        each record: 3 × float32 LE
          [0] eShift (metres, add to TM easting)
          [1] nShift (metres, add to TM northing)
          [2] hShift (metres, subtract from ellipsoidal height → ODN)

Record index = n_idx × 701 + e_idx
Byte offset  = 16 + index × 12
```

float32 gives ~0.01 mm precision on the shift values; the Dart implementation
uses float64 (double) internally, so no precision is lost in computation.

## Step 3 — verify the binary (optional but recommended)

```
python3 scripts/ostn15_verify.py
```

Runs all 40 OS-supplied test vectors through the full transform pipeline
(TM projection + bilinear interpolation + shift application) and checks each
output against the OS reference to 1 mm tolerance. Expected output:

```
Loading binary ...
  Grid: 701 × 1251  (876,951 nodes)
40/40 points passed (tolerance 1mm).
All tests passed. Binary is valid.
```

## Step 4 — copy to the tablet SD card

Create the `GPS_Staff/` directory on the SD card and copy the binary:

```
GPS_Staff/ostn15_osgm15.bin   (10.0 MB)
```

The app scans `/storage/` at startup for this path on any mounted volume
(internal storage and removable SD card are both checked).

## App integration

`Ostn15Service` (`lib/services/ostn15.dart`) is a singleton that:

1. **`load()`** — call once at app startup; scans storage volumes, validates
   the binary header, and memory-maps the grid into a `ByteData` buffer.
   Returns `true` on success, `false` if the file is absent or invalid.
2. **`isAvailable`** — check before calling `transform`.
3. **`transform(latDeg, lonDeg, heightM)`** — returns `Ostn15Result` with
   `osgbEasting`, `osgbNorthing`, `odnHeight`, or `null` if the point is
   outside the Great Britain coverage area.

The transform is synchronous after loading (~microseconds per call):

```dart
await Ostn15Service.instance.load();

final r = Ostn15Service.instance.transform(lat, lon, h);
if (r != null) {
  print('E ${r.osgbEasting.toStringAsFixed(3)} '
        'N ${r.osgbNorthing.toStringAsFixed(3)} '
        'H ${r.odnHeight.toStringAsFixed(3)}');
}
```

## Algorithm

The transform follows the OS publication *"A Guide to Coordinate Systems in
Great Britain"* (B7 Transformation from ETRS89 to OSGB36/ODN):

1. **TM projection** — convert ETRS89 latitude/longitude to ETRS89 projected
   easting/northing on the GRS80 ellipsoid using National Grid projection
   parameters (same as OSGB36; the ellipsoids differ but the projection
   parameters are identical for this step).

2. **Grid cell lookup** — divide projected E/N by 1000 to find the SW corner
   cell indices `(ei, ni)`.

3. **Bilinear interpolation** — compute fractional offsets `t = (E_proj mod
   1000) / 1000` and `u = (N_proj mod 1000) / 1000`, then interpolate the
   three shift components across the four surrounding nodes (SW, SE, NE, NW).

4. **Apply shifts** — `OSGB36_E = E_proj + eShift`, `OSGB36_N = N_proj +
   nShift`, `ODN_h = h_ellipsoidal − hShift`.

## Unit tests

```
flutter test test/ostn15_test.dart
```

Four TM projection spot-checks run unconditionally. The 40-point integration
test runs if the binary is on the device and skips silently otherwise
(CI-safe).
