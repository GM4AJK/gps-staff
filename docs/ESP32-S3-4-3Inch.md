# Waveshare ESP32-S3-Touch-LCD-4.3

## Resources

- **Demo code (zip):** https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-4.3/ESP32-S3-Touch-LCD-4.3-Demo.zip
- **Resources & documents:** https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3/Resources-And-Documents
- **ESP-IDF setup guide:** https://docs.waveshare.com/ESP32-S3-Touch-LCD-4.3/Development-Environment-Setup-ESP-IDF

## Hardware Specs

- ESP32-S3, dual-core 240MHz
- 512KB SRAM, 384KB ROM
- 8MB PSRAM (OCT mode)
- 16MB Flash
- 800×480 RGB LCD, 16-bit colour, 16MHz pixel clock
- GT911 capacitive touch controller (I2C)
- Two USB-C ports: **UART** (CH343P, use for flash/console) and **USB** (native OTG, GPIO19/20)

## USB / Console

The **UART** port connects via a QinHeng CH343P USB-serial chip (VID `1a86`, PID `55d3`).
Console goes through UART0 → CH343P. Use `CONFIG_ESP_CONSOLE_UART_DEFAULT` in
sdkconfig.defaults — **not** `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG` (that's for the Zeros).

WSL symlink: `/dev/esp32_handheld` (serial `5B79023040`, COM13 on Windows).

## Pin Assignments

### I2C (touch controller)

| Signal | GPIO |
|--------|------|
| SDA    | 8    |
| SCL    | 9    |

I2C port: `I2C_NUM_0`, 400kHz. GT911 address: `0x5D` (alt `0x14`).

### RGB LCD interface

| Signal | GPIO |
|--------|------|
| VSYNC  | 3    |
| HSYNC  | 46   |
| DE     | 5    |
| PCLK   | 7    |
| D0     | 14   |
| D1     | 38   |
| D2     | 18   |
| D3     | 17   |
| D4     | 10   |
| D5     | 39   |
| D6     | 0    |
| D7     | 45   |
| D8     | 48   |
| D9     | 47   |
| D10    | 21   |
| D11    | 1    |
| D12    | 2    |
| D13    | 42   |
| D14    | 41   |
| D15    | 40   |

RST and backlight: not wired (`-1`). Touch INT/RST: not wired (`-1`).

## LVGL Stack (example 08_lvgl_Porting)

Three components bundled locally (also available via IDF component registry):
- `espressif__esp_lcd_touch` — touch abstraction layer
- `espressif__esp_lcd_touch_gt911` — GT911 driver
- `lvgl__lvgl` — LVGL

Source files in `main/`:
- `waveshare_rgb_lcd_port.c/.h` — board init (LCD + touch), pin definitions
- `lvgl_port.c/.h` — LVGL tick/task, mutex helpers (`lvgl_port_lock` / `lvgl_port_unlock`)
- `main.c` — calls `waveshare_esp32_s3_rgb_lcd_init()` then LVGL demo

LVGL runs on **core 1** with tear-avoidance enabled.

## sdkconfig.defaults (from Waveshare example, IDF 5.2)

```
CONFIG_IDF_TARGET="esp32s3"
CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240=y
CONFIG_ESPTOOLPY_FLASHMODE_QIO=y
CONFIG_ESPTOOLPY_FLASHFREQ_80M=y
CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y
CONFIG_SPIRAM=y
CONFIG_SPIRAM_MODE_OCT=y
CONFIG_SPIRAM_RODATA=y
CONFIG_SPIRAM_SPEED_80M=y
CONFIG_FREERTOS_HZ=1000
CONFIG_ESP32S3_DATA_CACHE_LINE_64B=y
CONFIG_EXAMPLE_LVGL_PORT_TASK_CORE=1
CONFIG_EXAMPLE_LVGL_PORT_AVOID_TEAR_ENABLE=y
```

Note: Waveshare examples target IDF 5.2 / 5.5. We use IDF 6.2 — verify settings
compile cleanly and adjust if any Kconfig keys have changed.

## I2C Scan (factory firmware output)

Devices found on I2C bus at boot:
- `0x20–0x27`: likely PCF8574 GPIO expander range
- `0x30–0x3F`: unknown (possibly display controller responding to range)
- `0x5D`: GT911 touch controller
