# Handheld Unit — Implemented Capabilities

Covers `firmware/esp32-handheld`.

---

## ESP32-S3 4.3" Touch Display (firmware/esp32-handheld)

| Capability | Status | Notes |
|------------|--------|-------|
| RGB LCD display | Done PR #190 | 800×480, portrait via LVGL 270° rotation, 480×800 in software |
| GT911 touch | Done PR #191 | I2C GPIO8/9, addr 0x5D, bench verified |
| LVGL 8.4 stack | Done PR #190 | Tear-avoidance mode 3, core 1, lvgl_port_lock/unlock |
| Widget pattern | Done PR #199 | One widget per .c/.h, state in struct, timer context via user_data |
| Battery indicator widget | Done PR #199 | Charging animation, LV_SYMBOL_CHARGE bolt, batt_indicator.c/h |
| WiFi scanner widget | Done PR #200 | AP list sorted by RSSI, channel + auth type, 15s scan cycle (sandbox only — not in main build) |
| Partition table (1.5MB app) | Done PR #200 | SINGLE_APP_LARGE — sufficient for NimBLE + LVGL (792KB) |
| VS Code IntelliSense | Done PR #197 | c_cpp_properties.json → build/compile_commands.json |
| NimBLE central (BLE client) | Done PR #209 | Scans for GPS-Base, subscribes to 0xAC01 AP list, writes creds to 0xAC02 |
| WiFi provisioning UI | Done PR #209 | AP list from Base, password entry via LVGL keyboard, credential send; bench-verified |

---

## Pending / In Spec

| Feature | Spec |
|---------|------|
| BLE connection to Rover (position/status display) | Future spec |
| BLE command/config channel to Rover | Future spec |
| Main survey UI screens | Pending UI design |
