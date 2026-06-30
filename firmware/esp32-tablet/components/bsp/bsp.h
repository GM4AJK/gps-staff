#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_touch.h"
#include "driver/i2c_master.h"

// ── I2C ──────────────────────────────────────────────────────────────────────
#define BSP_I2C_PORT     I2C_NUM_0
#define BSP_I2C_SDA      GPIO_NUM_8
#define BSP_I2C_SCL      GPIO_NUM_9
#define BSP_I2C_FREQ_HZ  400000

// ── CH422G I/O expander (I2C addr 0x24) ──────────────────────────────────────
// Write format: [reg_byte, value_byte]
#define BSP_IO_EXP_ADDR      0x24
#define BSP_IO_EXP_REG_MODE  0x02   // set IO direction (0xFF = all output)
#define BSP_IO_EXP_REG_OUT   0x03   // write output levels
#define BSP_IO_EXP_REG_IN    0x04   // read input levels
#define BSP_IO_EXP_REG_PWM   0x05   // PWM brightness (0–255)

#define BSP_IO_TOUCH_RST  1   // IO_1 — GT911 reset
#define BSP_IO_BACKLIGHT  2   // IO_2 — backlight enable
#define BSP_IO_LCD_RST    3   // IO_3 — LCD reset
#define BSP_IO_SD_CS      4   // IO_4 — SD card CS

// ── RGB LCD panel ─────────────────────────────────────────────────────────────
#define BSP_LCD_H_RES         1024
#define BSP_LCD_V_RES          600
#define BSP_LCD_PIXEL_CLK_HZ  (30 * 1000 * 1000)

#define BSP_LCD_VSYNC   GPIO_NUM_3
#define BSP_LCD_HSYNC   GPIO_NUM_46
#define BSP_LCD_DE      GPIO_NUM_5
#define BSP_LCD_PCLK    GPIO_NUM_7
// Data bus B[3:7] G[2:7] R[3:7] — 16 bits
#define BSP_LCD_D0   GPIO_NUM_14   // B3
#define BSP_LCD_D1   GPIO_NUM_38   // B4
#define BSP_LCD_D2   GPIO_NUM_18   // B5
#define BSP_LCD_D3   GPIO_NUM_17   // B6
#define BSP_LCD_D4   GPIO_NUM_10   // B7
#define BSP_LCD_D5   GPIO_NUM_39   // G2
#define BSP_LCD_D6   GPIO_NUM_0    // G3
#define BSP_LCD_D7   GPIO_NUM_45   // G4
#define BSP_LCD_D8   GPIO_NUM_48   // G5
#define BSP_LCD_D9   GPIO_NUM_47   // G6
#define BSP_LCD_D10  GPIO_NUM_21   // G7
#define BSP_LCD_D11  GPIO_NUM_1    // R3
#define BSP_LCD_D12  GPIO_NUM_2    // R4
#define BSP_LCD_D13  GPIO_NUM_42   // R5
#define BSP_LCD_D14  GPIO_NUM_41   // R6
#define BSP_LCD_D15  GPIO_NUM_40   // R7

// ── GT911 touch ───────────────────────────────────────────────────────────────
#define BSP_TOUCH_I2C_ADDR  0x5D   // default when INT is low at power-up
#define BSP_TOUCH_INT       GPIO_NUM_4

// ── Public API ────────────────────────────────────────────────────────────────

// Bring up I2C, CH422G, LCD panel (with backlight on) and GT911 touch.
// panel_out and touch_out are mandatory output pointers.
esp_err_t bsp_init(esp_lcd_panel_handle_t *panel_out,
                   esp_lcd_touch_handle_t *touch_out);

// Get the PSRAM framebuffer allocated by the RGB panel driver.
// For single-buffer mode (NUM_FBS 1): pass count=1, fb1 receives the pointer.
esp_err_t bsp_lcd_get_frame_buffer(esp_lcd_panel_handle_t panel,
                                   void **fb1);

// Backlight brightness: 0 = off, 100 = full.
// Uses CH422G PWM register; values > 97 are clamped to 97 per Waveshare errata.
void bsp_backlight_set(uint8_t brightness_pct);
