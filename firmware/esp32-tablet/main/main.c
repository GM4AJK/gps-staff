#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "bsp.h"
#include "theme.h"

static const char *TAG = "main";

// ── LVGL tick ─────────────────────────────────────────────────────────────────

#define LVGL_TICK_MS 5

static void lvgl_tick_cb(void *arg)
{
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
    while (1) {
        uint32_t delay = lv_timer_handler();
        if (delay > 100) delay = 100;
        vTaskDelay(pdMS_TO_TICKS(delay));
    }
}

// ── First-light screen ────────────────────────────────────────────────────────

static esp_lcd_touch_handle_t s_touch;
static lv_obj_t *s_touch_label;

static void touch_read_cb(lv_timer_t *timer)
{
    (void)timer;
    esp_lcd_touch_read_data(s_touch);

    uint16_t x, y, strength;
    uint8_t  cnt = 0;
    bool     pressed = esp_lcd_touch_get_coordinates(s_touch, &x, &y, &strength, &cnt, 1);

    char buf[48];
    if (pressed && cnt > 0)
        snprintf(buf, sizeof(buf), "Touch  x=%4u  y=%4u", x, y);
    else
        snprintf(buf, sizeof(buf), "Touch  ---");
    lv_label_set_text(s_touch_label, buf);
}

static void build_first_light_screen(const app_theme_t *t)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, tc(t->bg_primary), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // Title
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "GPS Staff");
    lv_obj_set_style_text_color(title, tc(t->text_primary), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -60);

    // Subtitle
    lv_obj_t *sub = lv_label_create(scr);
    lv_label_set_text(sub, "First Light");
    lv_obj_set_style_text_color(sub, tc(t->text_muted), LV_PART_MAIN);
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(sub, LV_ALIGN_CENTER, 0, -20);

    // Touch readout
    s_touch_label = lv_label_create(scr);
    lv_label_set_text(s_touch_label, "Touch  ---");
    lv_obj_set_style_text_color(s_touch_label, tc(t->accent), LV_PART_MAIN);
    lv_obj_set_style_text_font(s_touch_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_align(s_touch_label, LV_ALIGN_CENTER, 0, 40);

    // Poll touch every 50 ms via LVGL timer
    lv_timer_create(touch_read_cb, 50, NULL);
}

// ── app_main ──────────────────────────────────────────────────────────────────

void app_main(void)
{
    ESP_LOGI(TAG, "GPS Staff esp32-tablet starting");

    // ── Hardware bring-up ──────────────────────────────────────────────────
    esp_lcd_panel_handle_t panel;
    ESP_ERROR_CHECK(bsp_init(&panel, &s_touch));

    // ── Get the PSRAM framebuffer from the RGB panel driver ────────────────
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

    // ── Build first-light UI ───────────────────────────────────────────────
    build_first_light_screen(&theme_dark);

    // ── LVGL handler task ─────────────────────────────────────────────────
    xTaskCreate(lvgl_task, "lvgl", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "Running");
}
