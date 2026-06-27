#include "nvs_flash.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "waveshare_rgb_lcd_port.h"
#include "batt_indicator.h"
#include "home_screen.h"
#include "base_config.h"
#include "wifi_provision.h"
#include "ble_base_client.h"

static batt_indicator_t batt;
static wifi_provision_t prov;

static void on_base_config_back(void)
{
	base_config_hide();
	home_screen_show();
}

static void on_home_button(bool is_base)
{
	if (!is_base) return;   /* Rover: TBD */
	home_screen_hide();
	base_config_show();
}

static void on_ap_list(const prov_ap_t *aps, uint8_t count)
{
	wifi_provision_update(&prov, aps, count);
}

static void on_conn_result(uint8_t status, const char *ssid, int8_t rssi, uint32_t ip)
{
	wifi_provision_on_result(&prov, status, ssid, rssi, ip);
}

static void on_ble_sync(void)
{
	ble_base_client_on_sync();
}

static void ble_host_task(void *arg)
{
	nimble_port_run();
	nimble_port_freertos_deinit();
}

void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		nvs_flash_erase();
		nvs_flash_init();
	}

	ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

	if (lvgl_port_lock(-1)) {
		lv_obj_t *scr = lv_scr_act();
		lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

		/* Create panels deepest-first; batt_indicator last so it floats on top */
		home_screen_init(scr, on_home_button);
		base_config_init(scr, &prov, on_base_config_back);
		wifi_provision_init(scr, &prov);
		batt_indicator_init(scr, &batt);

		home_screen_show();
		lvgl_port_unlock();
	}

	ble_base_client_init(on_ap_list);
	ble_base_client_set_result_cb(on_conn_result);
	nimble_port_init();
	ble_svc_gap_device_name_set("GPS-Handheld");
	ble_svc_gap_init();
	ble_svc_gatt_init();
	ble_hs_cfg.sync_cb = on_ble_sync;
	nimble_port_freertos_init(ble_host_task);
}
