#include "waveshare_rgb_lcd_port.h"

static lv_obj_t *batt_fill;
static int charge_pct = 10;

static void charging_anim_cb(lv_timer_t *timer)
{
    charge_pct += 10;
    if (charge_pct > 100)
        charge_pct = 10;
    lv_obj_set_width(batt_fill, lv_pct(charge_pct));
}

static void battery_indicator(lv_obj_t *parent)
{
    lv_obj_t *body = lv_obj_create(parent);
    lv_obj_set_size(body, 60, 26);
    lv_obj_align(body, LV_ALIGN_TOP_RIGHT, -18, 10);
    lv_obj_set_scrollbar_mode(body, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(body, lv_color_black(), 0);
    lv_obj_set_style_border_color(body, lv_color_white(), 0);
    lv_obj_set_style_border_width(body, 2, 0);
    lv_obj_set_style_radius(body, 3, 0);
    lv_obj_set_style_pad_all(body, 3, 0);

    batt_fill = lv_obj_create(body);
    lv_obj_set_height(batt_fill, lv_pct(100));
    lv_obj_set_width(batt_fill, lv_pct(charge_pct));
    lv_obj_align(batt_fill, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_scrollbar_mode(batt_fill, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(batt_fill, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_border_width(batt_fill, 0, 0);
    lv_obj_set_style_radius(batt_fill, 1, 0);

    lv_obj_t *nub = lv_obj_create(parent);
    lv_obj_set_size(nub, 5, 12);
    lv_obj_align(nub, LV_ALIGN_TOP_RIGHT, -12, 17);
    lv_obj_set_style_bg_color(nub, lv_color_white(), 0);
    lv_obj_set_style_border_width(nub, 0, 0);
    lv_obj_set_style_radius(nub, 2, 0);

    lv_timer_create(charging_anim_cb, 600, NULL);
}

void app_main(void)
{
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    if (lvgl_port_lock(-1)) {
        lv_obj_t *scr = lv_scr_act();
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
        battery_indicator(scr);
        lvgl_port_unlock();
    }
}
