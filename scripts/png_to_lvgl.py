#!/usr/bin/env python3
"""Convert a PNG to an LVGL 8 lv_img_dsc_t C source/header pair (RGB565, no alpha).

Usage:
    png_to_lvgl.py <input.png> <width> <height> <symbol_name> <output_dir>

Outputs <output_dir>/<symbol_name>.c and <output_dir>/<symbol_name>.h
"""
import sys
import os
from PIL import Image


def main():
    if len(sys.argv) != 6:
        print(__doc__)
        sys.exit(1)

    src, w_str, h_str, sym, out_dir = sys.argv[1:]
    w, h = int(w_str), int(h_str)

    img = Image.open(src).convert("RGB").resize((w, h), Image.LANCZOS)

    data = []
    for y in range(h):
        for x in range(w):
            r, g, b = img.getpixel((x, y))
            rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            data.append(rgb565 & 0xFF)        # little-endian low byte
            data.append((rgb565 >> 8) & 0xFF) # little-endian high byte

    os.makedirs(out_dir, exist_ok=True)
    c_path = os.path.join(out_dir, f"{sym}.c")
    h_path = os.path.join(out_dir, f"{sym}.h")

    with open(c_path, "w") as f:
        f.write(f"/* Auto-generated — do not edit. */\n")
        f.write(f"/* Source: {os.path.basename(src)}, scaled to {w}x{h} RGB565 */\n")
        f.write(f"#include \"lvgl.h\"\n\n")
        f.write(f"static const uint8_t _{sym}_data[] = {{\n")
        for i, byte in enumerate(data):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{byte:02x},")
            if i % 16 == 15:
                f.write("\n")
            else:
                f.write(" ")
        if len(data) % 16 != 0:
            f.write("\n")
        f.write("};\n\n")
        f.write(f"const lv_img_dsc_t {sym} = {{\n")
        f.write(f"    .header.cf          = LV_IMG_CF_TRUE_COLOR,\n")
        f.write(f"    .header.always_zero = 0,\n")
        f.write(f"    .header.reserved    = 0,\n")
        f.write(f"    .header.w           = {w},\n")
        f.write(f"    .header.h           = {h},\n")
        f.write(f"    .data_size          = {len(data)},\n")
        f.write(f"    .data               = _{sym}_data,\n")
        f.write(f"}};\n")

    with open(h_path, "w") as f:
        guard = f"{sym.upper()}_H"
        f.write(f"/* Auto-generated — do not edit. */\n")
        f.write(f"#pragma once\n")
        f.write(f"#include \"lvgl.h\"\n\n")
        f.write(f"extern const lv_img_dsc_t {sym};\n")

    print(f"Generated {c_path} and {h_path} ({len(data)} bytes, {w}x{h})")


if __name__ == "__main__":
    main()
