#include "waveshare_rgb_lcd_port.h"

void app_main(void)
{
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    if (lvgl_port_lock(-1)) {
        lv_obj_t *scr = lv_scr_act();

        lv_obj_t *label = lv_label_create(scr);
        lv_label_set_text(label, "GPS Staff");
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

        lvgl_port_unlock();
    }
}
