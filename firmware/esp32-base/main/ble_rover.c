#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"

#define TAG            "rover"
#define TARGET_NAME    "GPS-Base"
#define RTCM_SVC_UUID  0xAB00
#define RTCM_CHR_UUID  0xAB01
#define CCCD_UUID16    0x2902

static struct {
	uart_port_t uart;
	uint16_t    conn_handle;
	uint16_t    svc_start;
	uint16_t    svc_end;
	uint16_t    chr_val_h;
} s = { .conn_handle = BLE_HS_CONN_HANDLE_NONE };

static int gap_event_cb(struct ble_gap_event *event, void *arg);

static void start_scan(void)
{
	if (ble_gap_disc_active())
		ble_gap_disc_cancel();
	struct ble_gap_disc_params p = { .passive = 0, .filter_duplicates = 1 };
	ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &p, gap_event_cb, NULL);
	ESP_LOGI(TAG, "scanning for %s...", TARGET_NAME);
}

/* ---- GATT discovery callbacks ---- */

static int cccd_write_cb(uint16_t conn_h, const struct ble_gatt_error *err,
                          struct ble_gatt_attr *attr, void *arg)
{
	if (err->status == 0)
		ESP_LOGI(TAG, "subscribed");
	else
		ESP_LOGE(TAG, "CCCD write failed rc=%d", err->status);
	return 0;
}

static int dsc_cb(uint16_t conn_h, const struct ble_gatt_error *err,
                   uint16_t chr_val_h, const struct ble_gatt_dsc *dsc, void *arg)
{
	if (err->status == BLE_HS_EDONE)
		return 0;
	if (err->status != 0) {
		ESP_LOGE(TAG, "dsc disc rc=%d", err->status);
		return 0;
	}
	if (ble_uuid_u16(&dsc->uuid.u) == CCCD_UUID16) {
		uint16_t val = htobs(0x0001);
		ble_gattc_write_flat(conn_h, dsc->handle, &val, sizeof(val),
		                     cccd_write_cb, NULL);
	}
	return 0;
}

static int chr_cb(uint16_t conn_h, const struct ble_gatt_error *err,
                   const struct ble_gatt_chr *chr, void *arg)
{
	if (err->status == BLE_HS_EDONE) {
		if (s.chr_val_h)
			ble_gattc_disc_all_dscs(conn_h, s.chr_val_h + 1, s.svc_end,
			                        dsc_cb, NULL);
		return 0;
	}
	if (err->status != 0) {
		ESP_LOGE(TAG, "chr disc rc=%d", err->status);
		return 0;
	}
	if (ble_uuid_u16(&chr->uuid.u) == RTCM_CHR_UUID)
		s.chr_val_h = chr->val_handle;
	return 0;
}

static const ble_uuid16_t s_rtcm_svc_uuid = BLE_UUID16_INIT(RTCM_SVC_UUID);

static int svc_cb(uint16_t conn_h, const struct ble_gatt_error *err,
                   const struct ble_gatt_svc *svc, void *arg)
{
	if (err->status == BLE_HS_EDONE) {
		if (s.svc_start)
			ble_gattc_disc_all_chrs(conn_h, s.svc_start, s.svc_end,
			                        chr_cb, NULL);
		return 0;
	}
	if (err->status != 0) {
		ESP_LOGE(TAG, "svc disc rc=%d", err->status);
		return 0;
	}
	if (ble_uuid_u16(&svc->uuid.u) == RTCM_SVC_UUID) {
		s.svc_start = svc->start_handle;
		s.svc_end   = svc->end_handle;
	}
	return 0;
}

/* ---- GAP event handler ---- */

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
	switch (event->type) {

	case BLE_GAP_EVENT_DISC: {
		struct ble_hs_adv_fields fields;
		if (ble_hs_adv_parse_fields(&fields, event->disc.data,
		                             event->disc.length_data) != 0)
			break;
		if (!fields.name || fields.name_len != strlen(TARGET_NAME))
			break;
		if (memcmp(fields.name, TARGET_NAME, fields.name_len) != 0)
			break;
		ESP_LOGI(TAG, "found %s, connecting", TARGET_NAME);
		ble_gap_disc_cancel();
		ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &event->disc.addr,
		                BLE_HS_FOREVER, NULL, gap_event_cb, NULL);
		break;
	}

	case BLE_GAP_EVENT_CONNECT:
		if (event->connect.status == 0) {
			s.conn_handle = event->connect.conn_handle;
			ESP_LOGI(TAG, "connected handle=%d", s.conn_handle);
			ble_gattc_exchange_mtu(s.conn_handle, NULL, NULL);
			ble_gattc_disc_svc_by_uuid(s.conn_handle, &s_rtcm_svc_uuid.u,
			                           svc_cb, NULL);
		} else {
			ESP_LOGW(TAG, "connect failed, rescanning");
			start_scan();
		}
		break;

	case BLE_GAP_EVENT_DISCONNECT:
		ESP_LOGI(TAG, "disconnected reason=%d", event->disconnect.reason);
		s.conn_handle = BLE_HS_CONN_HANDLE_NONE;
		s.svc_start = s.svc_end = s.chr_val_h = 0;
		start_scan();
		break;

	case BLE_GAP_EVENT_MTU:
		ESP_LOGI(TAG, "MTU updated to %d", event->mtu.value);
		break;

	case BLE_GAP_EVENT_NOTIFY_RX:
		if (event->notify_rx.attr_handle == s.chr_val_h) {
			struct os_mbuf *om = event->notify_rx.om;
			uint16_t len = OS_MBUF_PKTLEN(om);
			uint8_t buf[512];
			if (len > sizeof(buf))
				len = sizeof(buf);
			os_mbuf_copydata(om, 0, len, buf);
			uart_write_bytes(s.uart, buf, len);
		}
		break;

	default:
		break;
	}
	return 0;
}

void ble_rover_init(uart_port_t uart)
{
	s.uart = uart;
}

void ble_rover_on_sync(void)
{
	start_scan();
}
