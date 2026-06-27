#include <string.h>
#include "esp_log.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "wifi_prov.h"

#define TAG "base"

void ble_base_on_sync(void); /* forward decl for gap_event_cb */

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
	switch (event->type) {
	case BLE_GAP_EVENT_CONNECT:
		if (event->connect.status == 0) {
			ESP_LOGI(TAG, "connected handle=%d", event->connect.conn_handle);
			wifi_prov_on_connect(event->connect.conn_handle);
		} else {
			ble_base_on_sync();
		}
		break;
	case BLE_GAP_EVENT_DISCONNECT:
		ESP_LOGI(TAG, "disconnected reason=%d", event->disconnect.reason);
		wifi_prov_on_disconnect();
		ble_base_on_sync();
		break;
	case BLE_GAP_EVENT_MTU:
		ESP_LOGI(TAG, "MTU updated to %d", event->mtu.value);
		break;
	default:
		break;
	}
	return 0;
}

void ble_base_on_sync(void)
{
	if (ble_gap_adv_active())
		ble_gap_adv_stop();

	const char *name = ble_svc_gap_device_name();
	struct ble_hs_adv_fields f = {0};
	f.flags            = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
	f.name             = (uint8_t *)name;
	f.name_len         = strlen(name);
	f.name_is_complete = 1;
	ble_gap_adv_set_fields(&f);

	struct ble_gap_adv_params p = {0};
	p.conn_mode = BLE_GAP_CONN_MODE_UND;
	p.disc_mode = BLE_GAP_DISC_MODE_GEN;
	ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &p,
	                  gap_event_cb, NULL);
	ESP_LOGI(TAG, "advertising as %s", name);
}
