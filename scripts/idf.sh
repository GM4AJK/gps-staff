#!/bin/bash
# Wrapper for idf.py with ESP-IDF 6.2 environment pre-set.
# Call as: scripts/idf.sh [idf.py args...]
# e.g.:   scripts/idf.sh build
#         scripts/idf.sh -p /dev/esp32_handheld flash

export IDF_PATH="/home/gm4ajk/esp/esp-idf"
export ESP_IDF_VERSION="6.2"
export IDF_PYTHON_ENV_PATH="/home/gm4ajk/.espressif/python_env/idf6.2_py3.12_env"
export OPENOCD_SCRIPTS="/home/gm4ajk/.espressif/tools/openocd-esp32/v0.12.0-esp32-20260424/openocd-esp32/share/openocd/scripts"
export ESP_ROM_ELF_DIR="/home/gm4ajk/.espressif/tools/esp-rom-elfs/20241011/"

export PATH="\
/home/gm4ajk/esp/esp-idf/components/espcoredump:\
/home/gm4ajk/esp/esp-idf/components/partition_table:\
/home/gm4ajk/esp/esp-idf/components/app_update:\
/home/gm4ajk/.espressif/tools/xtensa-esp-elf-gdb/17.1_20260402/xtensa-esp-elf-gdb/bin:\
/home/gm4ajk/.espressif/tools/xtensa-esp-elf/esp-15.2.0_20251204/xtensa-esp-elf/bin:\
/home/gm4ajk/.espressif/tools/riscv32-esp-elf/esp-15.2.0_20251204/riscv32-esp-elf/bin:\
/home/gm4ajk/.espressif/tools/esp32ulp-elf/2.38_20240113/esp32ulp-elf/bin:\
/home/gm4ajk/.espressif/tools/openocd-esp32/v0.12.0-esp32-20260424/openocd-esp32/bin:\
/home/gm4ajk/.espressif/tools/esp-clangd/esp-21.1.3_20260408/esp-clangd/bin:\
/home/gm4ajk/.espressif/tools/esp-idf-configdep/0.2.3/esp-idf-configdep-0.2.3/bin:\
/home/gm4ajk/.espressif/python_env/idf6.2_py3.12_env/bin:\
/home/gm4ajk/esp/esp-idf/tools:\
$PATH"

exec idf.py "$@"
