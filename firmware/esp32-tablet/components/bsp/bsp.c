#include "bsp.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lcd_touch_gt911.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "bsp";

// ── I2C bus ───────────────────────────────────────────────────────────────────

static i2c_master_bus_handle_t s_i2c_bus;

static esp_err_t i2c_init(void)
{
    i2c_master_bus_config_t cfg = {
        .clk_source        = I2C_CLK_SRC_DEFAULT,
        .i2c_port          = BSP_I2C_PORT,
        .scl_io_num        = BSP_I2C_SCL,
        .sda_io_num        = BSP_I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    return i2c_new_master_bus(&cfg, &s_i2c_bus);
}

// ── CH422G I/O expander ───────────────────────────────────────────────────────

static i2c_master_dev_handle_t s_io_exp;
static uint8_t s_io_outputs = 0xFF;   // shadow register; all high = safe default

static esp_err_t io_exp_write(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_io_exp, buf, sizeof(buf), 50);
}

static esp_err_t io_exp_init(void)
{
    i2c_device_config_t dev = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address  = BSP_IO_EXP_ADDR,
        .scl_speed_hz    = BSP_I2C_FREQ_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(s_i2c_bus, &dev, &s_io_exp),
                        TAG, "CH422G add device failed");
    s_io_outputs = 0xFF;
    return io_exp_write(BSP_IO_EXP_REG_MODE, s_io_outputs);   // all pins → output
}

static esp_err_t io_exp_set(uint8_t pin, bool high)
{
    if (high)
        s_io_outputs |= (uint8_t)(1u << pin);
    else
        s_io_outputs &= (uint8_t)~(1u << pin);
    return io_exp_write(BSP_IO_EXP_REG_OUT, s_io_outputs);
}

// ── Backlight ─────────────────────────────────────────────────────────────────

void bsp_backlight_set(uint8_t pct)
{
    if (pct == 0) {
        io_exp_set(BSP_IO_BACKLIGHT, false);
        return;
    }
    io_exp_set(BSP_IO_BACKLIGHT, true);
    if (pct > 97) pct = 97;   // Waveshare errata: 100% causes flicker
    uint8_t raw = (uint8_t)(pct * (255 / 100.0f));
    io_exp_write(BSP_IO_EXP_REG_PWM, raw);
}

// ── RGB LCD panel ─────────────────────────────────────────────────────────────

static esp_lcd_panel_handle_t s_panel;

static esp_err_t lcd_init(void)
{
    // Hold LCD in reset, wait, release
    io_exp_set(BSP_IO_LCD_RST, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    io_exp_set(BSP_IO_LCD_RST, true);
    vTaskDelay(pdMS_TO_TICKS(10));

    esp_lcd_rgb_panel_config_t cfg = {
        .clk_src  = LCD_CLK_SRC_DEFAULT,
        .timings  = {
            .pclk_hz           = BSP_LCD_PIXEL_CLK_HZ,
            .h_res             = BSP_LCD_H_RES,
            .v_res             = BSP_LCD_V_RES,
            .hsync_pulse_width = 162,
            .hsync_back_porch  = 152,
            .hsync_front_porch = 48,
            .vsync_pulse_width = 45,
            .vsync_back_porch  = 13,
            .vsync_front_porch = 3,
            .flags.pclk_active_neg = 1,
        },
        .data_width            = 16,
        .num_fbs               = 1,
        .bounce_buffer_size_px = BSP_LCD_H_RES * 10,
        .hsync_gpio_num        = BSP_LCD_HSYNC,
        .vsync_gpio_num        = BSP_LCD_VSYNC,
        .de_gpio_num           = BSP_LCD_DE,
        .pclk_gpio_num         = BSP_LCD_PCLK,
        .disp_gpio_num         = GPIO_NUM_NC,
        .data_gpio_nums = {
            BSP_LCD_D0,  BSP_LCD_D1,  BSP_LCD_D2,  BSP_LCD_D3,  BSP_LCD_D4,
            BSP_LCD_D5,  BSP_LCD_D6,  BSP_LCD_D7,  BSP_LCD_D8,  BSP_LCD_D9,
            BSP_LCD_D10, BSP_LCD_D11, BSP_LCD_D12, BSP_LCD_D13, BSP_LCD_D14,
            BSP_LCD_D15,
        },
        .flags.fb_in_psram = 1,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_rgb_panel(&cfg, &s_panel),
                        TAG, "RGB panel create failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(s_panel),
                        TAG, "RGB panel init failed");
    return ESP_OK;
}

esp_err_t bsp_lcd_get_frame_buffer(esp_lcd_panel_handle_t panel, void **fb1)
{
    return esp_lcd_rgb_panel_get_frame_buffer(panel, 1, fb1);
}

// ── GT911 touch ───────────────────────────────────────────────────────────────

static esp_err_t touch_init(esp_lcd_touch_handle_t *touch_out)
{
    // Release touch reset via CH422G before initialising
    io_exp_set(BSP_IO_TOUCH_RST, false);
    vTaskDelay(pdMS_TO_TICKS(10));
    io_exp_set(BSP_IO_TOUCH_RST, true);
    vTaskDelay(pdMS_TO_TICKS(50));

    esp_lcd_panel_io_i2c_config_t io_cfg = {
        .dev_addr            = BSP_TOUCH_I2C_ADDR,
        .control_phase_bytes = 1,
        .lcd_cmd_bits        = 16,
        .lcd_param_bits      = 8,
        .dc_bit_offset       = 0,
        .scl_speed_hz        = BSP_I2C_FREQ_HZ,
        .flags.disable_control_phase = 1,
    };
    esp_lcd_panel_io_handle_t tp_io;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(s_i2c_bus, &io_cfg, &tp_io),
                        TAG, "Touch IO create failed");

    esp_lcd_touch_config_t tp_cfg = {
        .x_max       = BSP_LCD_H_RES,
        .y_max       = BSP_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,   // reset done via CH422G above
        .int_gpio_num = BSP_TOUCH_INT,
        .levels.reset     = 0,
        .levels.interrupt = 0,
        .flags.swap_xy    = 0,
        .flags.mirror_x   = 0,
        .flags.mirror_y   = 0,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_touch_new_i2c_gt911(tp_io, &tp_cfg, touch_out),
                        TAG, "GT911 init failed");
    return ESP_OK;
}

// ── Public init ───────────────────────────────────────────────────────────────

esp_err_t bsp_init(esp_lcd_panel_handle_t *panel_out,
                   esp_lcd_touch_handle_t *touch_out)
{
    ESP_RETURN_ON_ERROR(i2c_init(),      TAG, "I2C init failed");
    ESP_RETURN_ON_ERROR(io_exp_init(),   TAG, "IO expander init failed");
    ESP_RETURN_ON_ERROR(lcd_init(),      TAG, "LCD init failed");
    ESP_RETURN_ON_ERROR(touch_init(touch_out), TAG, "Touch init failed");

    bsp_backlight_set(80);   // backlight on at 80% — safe for first boot

    *panel_out = s_panel;
    ESP_LOGI(TAG, "BSP ready — %dx%d, backlight 80%%", BSP_LCD_H_RES, BSP_LCD_V_RES);
    return ESP_OK;
}
