#include "waveshare_rgb_lcd_port.h"

static lv_obj_t *coord_label;

static void touch_event_cb(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_get_act();
    lv_point_t p;
    lv_indev_get_point(indev, &p);

    if (lvgl_port_lock(0)) {
        lv_label_set_text_fmt(coord_label, "Touch: %d, %d", p.x, p.y);
        lvgl_port_unlock();
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    if (lvgl_port_lock(-1)) {
        lv_obj_t *scr = lv_scr_act();
        lv_obj_add_event_cb(scr, touch_event_cb, LV_EVENT_PRESSING, NULL);
        lv_obj_add_flag(scr, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_t *title = lv_label_create(scr);
        lv_label_set_text(title, "GPS Staff");
        lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
        lv_obj_align(title, LV_ALIGN_CENTER, 0, -40);

        coord_label = lv_label_create(scr);
        lv_label_set_text(coord_label, "Touch anywhere...");
        lv_obj_set_style_text_font(coord_label, &lv_font_montserrat_24, 0);
        lv_obj_align(coord_label, LV_ALIGN_CENTER, 0, 40);

        lvgl_port_unlock();
    }
}
