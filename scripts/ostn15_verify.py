#!/usr/bin/env python3
"""
Verify ostn15_osgm15.bin against the OS-supplied test vectors.

Implements the full ETRS89 (lat, lon, h) → OSGB36 (E, N) + ODN transform:
  1. TM projection  (ETRS89 lat/lon → ETRS89 projected E/N on GRS80)
  2. Grid lookup + bilinear interpolation from the binary
  3. Apply shifts

Run from repo root:
  python3 scripts/ostn15_verify.py
"""

import math
import os
import struct

BIN_FILE   = 'tmp/ostn15_osgm15/ostn15_osgm15.bin'
TEST_IN    = 'tmp/ostn15_osgm15/OSTN15_OSGM15_TestInput_ETRStoOSGB.txt'
TEST_OUT   = 'tmp/ostn15_osgm15/OSTN15_OSGM15_TestOutput_ETRStoOSGB.txt'

MAGIC    = b'OSTN1500'
NUM_COLS = 701

# ── GRS80 / National Grid constants ──────────────────────────────────────────

a   = 6_378_137.000            # GRS80 semi-major axis (m)
f   = 1.0 / 298.257_222_101   # GRS80 flattening
b   = a * (1.0 - f)           # semi-minor axis
F0  = 0.999_601_2717          # scale factor on central meridian
phi0 = math.radians(49.0)     # true origin latitude
lam0 = math.radians(-2.0)     # true origin longitude
E0  = 400_000.0               # false easting (m)
N0  = -100_000.0              # false northing (m)

e2  = (a*a - b*b) / (a*a)
n_  = (a - b) / (a + b)

# ── TM projection ─────────────────────────────────────────────────────────────

def _meridional_arc(phi):
    n2, n3 = n_*n_, n_*n_*n_
    dp, sp = phi - phi0, phi + phi0
    return b * F0 * (
        (1 + n_ + 5/4*n2 + 5/4*n3)          * dp
      - (3*n_ + 3*n2 + 21/8*n3)             * math.sin(dp)   * math.cos(sp)
      + (15/8*n2 + 15/8*n3)                 * math.sin(2*dp) * math.cos(2*sp)
      - 35/24*n3                             * math.sin(3*dp) * math.cos(3*sp)
    )

def etrs89_to_tm(lat_deg, lon_deg):
    """ETRS89 geographic → ETRS89 Transverse Mercator projected (m)."""
    phi = math.radians(lat_deg)
    lam = math.radians(lon_deg)

    s  = math.sin(phi)
    c  = math.cos(phi)
    t  = math.tan(phi)
    nu  = a * F0 / math.sqrt(1 - e2 * s*s)
    rho = a * F0 * (1 - e2) / (1 - e2 * s*s)**1.5
    eta2 = nu/rho - 1

    M  = _meridional_arc(phi)
    dl = lam - lam0

    N = (M + N0
         + nu/2  * s*c     * dl**2
         + nu/24 * s*c**3  * (5 - t**2 + 9*eta2)          * dl**4
         + nu/720* s*c**5  * (61 - 58*t**2 + t**4)         * dl**6)

    E = (E0
         + nu    * c       * dl
         + nu/6  * c**3    * (nu/rho - t**2)                * dl**3
         + nu/120* c**5    * (5 - 18*t**2 + t**4 + 14*eta2 - 58*t**2*eta2) * dl**5)

    return E, N

# ── Binary grid lookup ────────────────────────────────────────────────────────

def load_bin(path):
    with open(path, 'rb') as f:
        magic = f.read(8)
        if magic != MAGIC:
            raise ValueError(f'Bad magic: {magic}')
        cols, rows = struct.unpack('<II', f.read(8))
        data = f.read()
    expected = cols * rows * 12
    if len(data) != expected:
        raise ValueError(f'Data length {len(data)} != expected {expected}')
    return data, cols, rows

def _node(data, cols, ei, ni):
    idx = ni * cols + ei
    return struct.unpack_from('<fff', data, idx * 12)

def transform(lat_deg, lon_deg, h_m, data, cols, rows):
    """
    Full ETRS89 → OSGB36 / ODN transform.
    Returns (osgb_e, osgb_n, odn_h) or None if outside grid.
    """
    E_proj, N_proj = etrs89_to_tm(lat_deg, lon_deg)

    ei = int(E_proj / 1000)
    ni = int(N_proj / 1000)

    if ei < 0 or ei >= cols-1 or ni < 0 or ni >= rows-1:
        return None  # outside grid

    t = (E_proj - ei * 1000) / 1000
    u = (N_proj - ni * 1000) / 1000

    sw = _node(data, cols, ei,   ni)
    se = _node(data, cols, ei+1, ni)
    ne = _node(data, cols, ei+1, ni+1)
    nw = _node(data, cols, ei,   ni+1)

    def interp(i):
        return ((1-t)*(1-u)*sw[i] + t*(1-u)*se[i]
               + t*u*ne[i] + (1-t)*u*nw[i])

    se_shift = interp(0)
    sn_shift = interp(1)
    sg_shift = interp(2)

    return (E_proj + se_shift,
            N_proj + sn_shift,
            h_m    - sg_shift)

# ── Test runner ───────────────────────────────────────────────────────────────

def parse_inputs(path):
    pts = {}
    with open(path) as f:
        next(f)
        for line in f:
            p = line.strip().split(',')
            if len(p) < 4:
                continue
            pts[p[0]] = (float(p[1]), float(p[2]), float(p[3]))
    return pts

def parse_outputs(path):
    pts = {}
    with open(path) as f:
        next(f)
        for line in f:
            p = line.strip().split(',')
            if len(p) < 4:
                continue
            pts[p[0]] = (float(p[1]), float(p[2]), float(p[3]))
    return pts

def main():
    repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    print('Loading binary ...')
    data, cols, rows = load_bin(os.path.join(repo, BIN_FILE))
    print(f'  Grid: {cols} × {rows}  ({cols*rows:,} nodes)')

    inputs  = parse_inputs (os.path.join(repo, TEST_IN))
    outputs = parse_outputs(os.path.join(repo, TEST_OUT))

    passed = failed = 0
    for pt_id, (lat, lon, h) in sorted(inputs.items()):
        result = transform(lat, lon, h, data, cols, rows)
        if result is None:
            print(f'  {pt_id}: OUTSIDE GRID')
            failed += 1
            continue

        calc_e, calc_n, calc_h = result
        exp_e,  exp_n,  exp_h  = outputs[pt_id]

        de = abs(calc_e - exp_e)
        dn = abs(calc_n - exp_n)
        dh = abs(calc_h - exp_h)

        ok = de < 0.001 and dn < 0.001 and dh < 0.001
        status = 'PASS' if ok else 'FAIL'
        if ok:
            passed += 1
        else:
            failed += 1
            print(f'  {pt_id}: {status}  ΔE={de:.4f}  ΔN={dn:.4f}  Δh={dh:.4f}')

    print(f'\n{passed}/{passed+failed} points passed (tolerance 1mm).')
    if failed:
        print('FAILURES detected — check implementation.')
    else:
        print('All tests passed. Binary is valid.')

if __name__ == '__main__':
    main()
