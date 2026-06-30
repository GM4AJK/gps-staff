#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "bsp.h"
#include "theme.h"
#include "screen_home.h"
#include "screen_base_config.h"

static const char *TAG = "main";

// ── LVGL tick ─────────────────────────────────────────────────────────────────

#define LVGL_TICK_MS 5

static void lvgl_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(LVGL_TICK_MS);
}

// ── LVGL flush callback (single-buffer direct mode) ───────────────────────────
// The RGB peripheral DMA reads the framebuffer directly; nothing to transfer.

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    (void)area;
    (void)px_map;
    lv_display_flush_ready(disp);
}

// ── LVGL handler task ─────────────────────────────────────────────────────────

static void lvgl_task(void *arg)
{
    (void)arg;
    while (1) {
        uint32_t delay = lv_timer_handler();
        if (delay > 100) delay = 100;
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

// ── Touch input device ────────────────────────────────────────────────────────

static esp_lcd_touch_handle_t s_touch;

static void touch_input_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    esp_lcd_touch_read_data(s_touch);

    esp_lcd_touch_point_data_t pt;
    uint8_t cnt = 0;
    esp_err_t ret = esp_lcd_touch_get_data(s_touch, &pt, &cnt, 1);

    if (ret == ESP_OK && cnt > 0) {
        data->point.x = (int32_t)pt.x;
        data->point.y = (int32_t)pt.y;
        data->state   = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// ── app_main ──────────────────────────────────────────────────────────────────

void app_main(void)
{
    ESP_LOGI(TAG, "GPS Staff esp32-tablet starting");

    // ── Hardware bring-up ──────────────────────────────────────────────────
    esp_lcd_panel_handle_t panel;
    ESP_ERROR_CHECK(bsp_init(&panel, &s_touch));

    // ── PSRAM framebuffer ─────────────────────────────────────────────────
    void *fb = NULL;
    ESP_ERROR_CHECK(bsp_lcd_get_frame_buffer(panel, &fb));
    ESP_LOGI(TAG, "Framebuffer at %p (%u bytes)",
             fb, BSP_LCD_H_RES * BSP_LCD_V_RES * 2);

    // ── LVGL init ──────────────────────────────────────────────────────────
    lv_init();

    lv_display_t *disp = lv_display_create(BSP_LCD_H_RES, BSP_LCD_V_RES);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    lv_display_set_buffers(disp, fb, NULL,
                           BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(lv_color16_t),
                           LV_DISPLAY_RENDER_MODE_DIRECT);

    // LVGL tick via esp_timer
    const esp_timer_create_args_t tick_args = {
        .callback = lvgl_tick_cb,
        .name     = "lvgl_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, LVGL_TICK_MS * 1000));

    // ── Touch input device ─────────────────────────────────────────────────
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_input_cb);

    // ── Build screens (all at boot, in PSRAM) ─────────────────────────────
    lv_obj_t *home        = screen_home_create(&theme_dark);
    lv_obj_t *base_config = screen_base_config_create(&theme_dark);

    // Wire up navigation
    screen_home_set_nav_base(base_config);
    screen_base_config_set_nav_back(home);

    lv_scr_load(home);

    // ── LVGL handler task ─────────────────────────────────────────────────
    xTaskCreate(lvgl_task, "lvgl", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "Running");
}
