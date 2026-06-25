#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"

#define TAG            "base"
#define RTCM_SVC_UUID  0xAB00
#define RTCM_CHR_UUID  0xAB01

static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint16_t s_rtcm_val_handle;
static uart_port_t s_uart;

static int rtcm_access_cb(uint16_t conn_h, uint16_t attr_h,
                           struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	return BLE_ATT_ERR_READ_NOT_PERMITTED;
}

static const struct ble_gatt_svc_def s_svcs[] = {
	{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID16_DECLARE(RTCM_SVC_UUID),
		.characteristics = (struct ble_gatt_chr_def[]) {
			{
				.uuid       = BLE_UUID16_DECLARE(RTCM_CHR_UUID),
				.flags      = BLE_GATT_CHR_F_NOTIFY,
				.val_handle = &s_rtcm_val_handle,
				.access_cb  = rtcm_access_cb,
			},
			{ 0 }
		},
	},
	{ 0 }
};

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
	switch (event->type) {
	case BLE_GAP_EVENT_CONNECT:
		if (event->connect.status == 0) {
			s_conn_handle = event->connect.conn_handle;
			ESP_LOGI(TAG, "connected handle=%d", s_conn_handle);
		} else {
			ble_base_on_sync();
		}
		break;
	case BLE_GAP_EVENT_DISCONNECT:
		s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
		ESP_LOGI(TAG, "disconnected reason=%d", event->disconnect.reason);
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

void ble_base_register_svcs(void)
{
	ble_gatts_count_cfg(s_svcs);
	ble_gatts_add_svcs(s_svcs);
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
	ESP_LOGI(TAG, "advertising");
}

static void uart_bridge_task(void *arg)
{
	/* 500 bytes fits comfortably within the negotiated 512-byte ATT MTU.
	 * Notifications sent before MTU exchange completes will fail silently
	 * and log a warning; RTCM parsers on the Nucleo tolerate missing leading
	 * frames. */
	uint8_t buf[500];
	while (1) {
		int len = uart_read_bytes(s_uart, buf, sizeof(buf), pdMS_TO_TICKS(20));
		if (len <= 0 || s_conn_handle == BLE_HS_CONN_HANDLE_NONE)
			continue;

		struct os_mbuf *om = ble_hs_mbuf_from_flat(buf, len);
		if (!om)
			continue;
		int rc = ble_gatts_notify_custom(s_conn_handle, s_rtcm_val_handle, om);
		if (rc != 0)
			ESP_LOGW(TAG, "notify rc=%d len=%d", rc, len);
	}
}

void ble_base_start_tasks(uart_port_t uart)
{
	s_uart = uart;
	xTaskCreate(uart_bridge_task, "uart_bridge", 4096, NULL, 5, NULL);
}
