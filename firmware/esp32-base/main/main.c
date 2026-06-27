#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_base.h"
#include "wifi_prov.h"

#define TAG "main"

/* Mode select pins (GP1/GP2 on ESP32-S3 Mini, driven by F765 PE2/PE3 on PCB).
 * Bench: GP1 hardwired — link open = base (pull-up), link to GND = rover.
 *        GP2 not connected; boot times out and reads GP1 directly. */
#define PIN_MODE         GPIO_NUM_1
#define PIN_MODE_VALID   GPIO_NUM_2
#define VALID_TIMEOUT_MS 500

typedef enum { ROLE_BASE, ROLE_ROVER } role_t;

static role_t g_role;

static role_t detect_role(void)
{
	gpio_config_t gp1 = {
		.pin_bit_mask = 1ULL << PIN_MODE,
		.mode         = GPIO_MODE_INPUT,
		.pull_up_en   = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type    = GPIO_INTR_DISABLE,
	};
	gpio_config(&gp1);

	gpio_config_t gp2 = {
		.pin_bit_mask = 1ULL << PIN_MODE_VALID,
		.mode         = GPIO_MODE_INPUT,
		.pull_up_en   = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.intr_type    = GPIO_INTR_DISABLE,
	};
	gpio_config(&gp2);

	TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(VALID_TIMEOUT_MS);
	while (!gpio_get_level(PIN_MODE_VALID)) {
		if (xTaskGetTickCount() >= deadline) {
			ESP_LOGW(TAG, "GP2 timeout — bench mode");
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	role_t role = gpio_get_level(PIN_MODE) ? ROLE_BASE : ROLE_ROVER;
	ESP_LOGI(TAG, "role: %s", role == ROLE_BASE ? "BASE" : "ROVER");
	return role;
}

static void on_ble_sync(void)
{
	ble_hs_util_ensure_addr(0);
	if (g_role == ROLE_BASE)
		ble_base_on_sync();
	/* rover: BLE role TBD — RTCM flows over GFSK, not BLE */
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

	g_role = detect_role();

	nimble_port_init();
	ble_svc_gap_device_name_set(g_role == ROLE_BASE ? "GPS-Base" : "GPS-Rover");
	ble_svc_gap_init();
	ble_svc_gatt_init();

	if (g_role == ROLE_BASE) {
		esp_netif_init();
		esp_event_loop_create_default();
		wifi_prov_register_svcs();
		wifi_prov_init();
	}

	ble_hs_cfg.sync_cb = on_ble_sync;
	nimble_port_freertos_init(ble_host_task);

	if (g_role == ROLE_BASE)
		wifi_prov_start_task();

	ESP_LOGI(TAG, "init complete");
}
