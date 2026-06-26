#include "waveshare_rgb_lcd_port.h"

#include "batt_indicator.h"
#include "wifi_scanner.h"

static batt_indicator_t batt;
static wifi_scanner_t   wifi;

void app_main(void)
{
	ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

	if (lvgl_port_lock(-1)) {
		lv_obj_t *scr = lv_scr_act();
		lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
		batt_indicator_init(scr, &batt);
		wifi_scanner_init(scr, &wifi);
		lvgl_port_unlock();
	}
}

