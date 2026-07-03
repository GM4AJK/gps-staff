#!/usr/bin/env python3
"""
Convert OSTN15_OSGM15_DataFile.txt → ostn15_osgm15.bin (compact binary for SD card).

Binary layout (little-endian):
  [0..7]   magic   b'OSTN1500'
  [8..11]  uint32  num_cols = 701  (E: 0 to 700000 m, step 1000)
  [12..15] uint32  num_rows = 1251 (N: 0 to 1250000 m, step 1000)
  [16..]   records: 3 × float32 per node (eShift, nShift, hShift), row-major
             index = n_idx * num_cols + e_idx
             offset = 16 + index * 12

Run from repo root:
  python3 scripts/ostn15_convert.py
"""

import os
import struct

INPUT  = 'tmp/ostn15_osgm15/OSTN15_OSGM15_DataFile.txt'
OUTPUT = 'tmp/ostn15_osgm15/ostn15_osgm15.bin'

NUM_COLS = 701
NUM_ROWS = 1251
EXPECTED = NUM_COLS * NUM_ROWS  # 876,951
MAGIC    = b'OSTN1500'

def main():
    repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    inp  = os.path.join(repo, INPUT)
    out  = os.path.join(repo, OUTPUT)

    print(f'Reading {inp} ...')
    buf = bytearray(EXPECTED * 12)

    count = 0
    with open(inp, 'r') as f:
        next(f)  # skip header
        for line in f:
            parts = line.strip().split(',')
            if len(parts) < 6:
                continue
            idx     = int(parts[0]) - 1  # 0-based; file is already row-major
            e_shift = float(parts[3])
            n_shift = float(parts[4])
            h_shift = float(parts[5])
            struct.pack_into('<fff', buf, idx * 12, e_shift, n_shift, h_shift)
            count += 1
            if count % 200_000 == 0:
                pct = 100 * count // EXPECTED
                print(f'  {count:,}/{EXPECTED:,}  ({pct}%)')

    if count != EXPECTED:
        raise RuntimeError(f'Expected {EXPECTED} records, got {count}')

    with open(out, 'wb') as f:
        f.write(MAGIC)
        f.write(struct.pack('<II', NUM_COLS, NUM_ROWS))
        f.write(buf)

    size_mb = os.path.getsize(out) / 1_048_576
    print(f'Written {out}  ({size_mb:.1f} MB)')
    print('Copy this file to GPS_Staff/ostn15_osgm15.bin on the tablet SD card.')

if __name__ == '__main__':
    main()
