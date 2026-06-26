/*
 * Author: Andy Kirkham
 */
#include "wifi_scanner.h"
#include "lvgl_port.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_APS          20
#define SCAN_INTERVAL_MS 15000

static int rssi_cmp(const void *a, const void *b)
{
	return ((wifi_ap_record_t *)b)->rssi - ((wifi_ap_record_t *)a)->rssi;
}

static const char *auth_str(wifi_auth_mode_t mode)
{
	switch (mode) {
	case WIFI_AUTH_OPEN:         return "Open";
	case WIFI_AUTH_WEP:          return "WEP";
	case WIFI_AUTH_WPA_PSK:      return "WPA";
	case WIFI_AUTH_WPA2_PSK:     return "WPA2";
	case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/2";
	case WIFI_AUTH_WPA3_PSK:     return "WPA3";
	case WIFI_AUTH_WPA2_WPA3_PSK:return "WPA2/3";
	default:                     return "Ent";
	}
}

static void scan_task(void *arg)
{
	wifi_scanner_t *w = (wifi_scanner_t *)arg;
	char buf[80];

	for (;;) {
		if (lvgl_port_lock(-1)) {
			lv_label_set_text(w->status, LV_SYMBOL_REFRESH " Scanning...");
			lvgl_port_unlock();
		}

		esp_wifi_scan_start(NULL, true);

		uint16_t ap_count = MAX_APS;
		wifi_ap_record_t aps[MAX_APS];
		esp_wifi_scan_get_ap_records(&ap_count, aps);

		qsort(aps, ap_count, sizeof(wifi_ap_record_t), rssi_cmp);

		if (lvgl_port_lock(-1)) {
			lv_obj_clean(w->list);

			for (int i = 0; i < ap_count; i++) {
				snprintf(buf, sizeof(buf), "%-20.20s  %4d dBm  ch%-3d %s",
				         (char *)aps[i].ssid,
				         aps[i].rssi,
				         aps[i].primary,
				         auth_str(aps[i].authmode));
				lv_obj_t *btn = lv_list_add_btn(w->list, LV_SYMBOL_WIFI, buf);
				lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
				lv_obj_set_style_text_color(btn, lv_color_white(), 0);
			}

			snprintf(buf, sizeof(buf), LV_SYMBOL_WIFI " %d networks found", ap_count);
			lv_label_set_text(w->status, buf);

			lvgl_port_unlock();
		}

		vTaskDelay(pdMS_TO_TICKS(SCAN_INTERVAL_MS));
	}
}

void wifi_scanner_init(lv_obj_t *parent, wifi_scanner_t *w)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		nvs_flash_erase();
		nvs_flash_init();
	}

	esp_netif_init();
	esp_event_loop_create_default();
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_set_mode(WIFI_MODE_STA);
	esp_wifi_start();

	lv_obj_t *title = lv_label_create(parent);
	lv_label_set_text(title, "WiFi Networks");
	lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
	lv_obj_set_style_text_color(title, lv_color_white(), 0);
	lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

	w->status = lv_label_create(parent);
	lv_label_set_text(w->status, "Starting...");
	lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_GREY), 0);
	lv_obj_align(w->status, LV_ALIGN_TOP_MID, 0, 46);

	w->list = lv_list_create(parent);
	lv_obj_set_pos(w->list, 0, 80);
	lv_obj_set_size(w->list, lv_pct(100), 720);
	lv_obj_set_style_bg_color(w->list, lv_color_black(), 0);
	lv_obj_set_style_border_width(w->list, 0, 0);
	lv_obj_set_style_radius(w->list, 0, 0);

	xTaskCreate(scan_task, "wifi_scan", 4096, w, 5, NULL);
}
