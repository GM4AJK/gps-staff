#include <string.h>
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "ble_base_client.h"

#define TAG           "ble_client"
#define TARGET_NAME   "GPS-Base"
#define PROV_SVC_UUID 0xAC00
#define AP_LIST_UUID  0xAC01
#define CRED_UUID     0xAC02
#define CCCD_UUID16   0x2902

static struct {
	uint16_t          conn_handle;
	uint16_t          svc_start, svc_end;
	uint16_t          ap_list_val_h;
	uint16_t          cred_val_h;
	prov_ap_list_cb_t ap_cb;
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

/* ---- GATT callbacks ---- */

static int cred_write_cb(uint16_t conn_h, const struct ble_gatt_error *err,
                          struct ble_gatt_attr *attr, void *arg)
{
	if (err->status == 0)
		ESP_LOGI(TAG, "credentials acknowledged by Base");
	else
		ESP_LOGE(TAG, "credential write failed rc=%d", err->status);
	return 0;
}

static int cccd_write_cb(uint16_t conn_h, const struct ble_gatt_error *err,
                          struct ble_gatt_attr *attr, void *arg)
{
	if (err->status == 0)
		ESP_LOGI(TAG, "subscribed to AP list");
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
		uint16_t val = 0x0001;
		ble_gattc_write_flat(conn_h, dsc->handle, &val, sizeof(val),
		                     cccd_write_cb, NULL);
	}
	return 0;
}

static int chr_cb(uint16_t conn_h, const struct ble_gatt_error *err,
                   const struct ble_gatt_chr *chr, void *arg)
{
	if (err->status == BLE_HS_EDONE) {
		if (s.ap_list_val_h)
			ble_gattc_disc_all_dscs(conn_h, s.ap_list_val_h, s.svc_end,
			                        dsc_cb, NULL);
		return 0;
	}
	if (err->status != 0) {
		ESP_LOGE(TAG, "chr disc rc=%d", err->status);
		return 0;
	}
	uint16_t u = ble_uuid_u16(&chr->uuid.u);
	if (u == AP_LIST_UUID)
		s.ap_list_val_h = chr->val_handle;
	else if (u == CRED_UUID)
		s.cred_val_h = chr->val_handle;
	return 0;
}

static const ble_uuid16_t s_prov_svc_uuid = BLE_UUID16_INIT(PROV_SVC_UUID);

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
	if (ble_uuid_u16(&svc->uuid.u) == PROV_SVC_UUID) {
		s.svc_start = svc->start_handle;
		s.svc_end   = svc->end_handle;
	}
	return 0;
}

/* ---- AP list decode ---- */

static void decode_ap_list(const uint8_t *data, size_t len)
{
	if (!s.ap_cb || len < 1)
		return;
	uint8_t count = data[0];
	prov_ap_t aps[20];
	uint8_t n = 0;
	const uint8_t *p   = data + 1;
	const uint8_t *end = data + len;
	while (n < count && n < 20 && p < end) {
		uint8_t slen = *p++;
		if (p + slen + 2 > end)
			break;
		memcpy(aps[n].ssid, p, slen);
		aps[n].ssid[slen] = '\0';
		p += slen;
		aps[n].rssi = (int8_t)*p++;
		aps[n].auth = *p++;
		n++;
	}
	s.ap_cb(aps, n);
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
			ble_gattc_disc_svc_by_uuid(s.conn_handle, &s_prov_svc_uuid.u,
			                           svc_cb, NULL);
		} else {
			ESP_LOGW(TAG, "connect failed, rescanning");
			start_scan();
		}
		break;

	case BLE_GAP_EVENT_DISCONNECT:
		ESP_LOGI(TAG, "disconnected reason=%d", event->disconnect.reason);
		s.conn_handle   = BLE_HS_CONN_HANDLE_NONE;
		s.svc_start     = s.svc_end = 0;
		s.ap_list_val_h = s.cred_val_h = 0;
		start_scan();
		break;

	case BLE_GAP_EVENT_MTU:
		ESP_LOGI(TAG, "MTU updated to %d", event->mtu.value);
		break;

	case BLE_GAP_EVENT_NOTIFY_RX:
		if (event->notify_rx.attr_handle == s.ap_list_val_h) {
			uint8_t  buf[700];
			uint16_t len = OS_MBUF_PKTLEN(event->notify_rx.om);
			if (len > sizeof(buf))
				len = sizeof(buf);
			os_mbuf_copydata(event->notify_rx.om, 0, len, buf);
			decode_ap_list(buf, len);
		}
		break;

	default:
		break;
	}
	return 0;
}

void ble_base_client_init(prov_ap_list_cb_t cb)
{
	s.ap_cb = cb;
}

void ble_base_client_on_sync(void)
{
	ble_hs_util_ensure_addr(0);
	start_scan();
}

void ble_base_client_send_creds(const char *ssid, const char *password)
{
	if (s.conn_handle == BLE_HS_CONN_HANDLE_NONE || !s.cred_val_h) {
		ESP_LOGW(TAG, "not connected, cannot send creds");
		return;
	}
	uint8_t buf[1 + 32 + 1 + 64];
	uint8_t *p    = buf;
	uint8_t  slen = (uint8_t)strlen(ssid);
	uint8_t  plen = (uint8_t)strlen(password);
	*p++ = slen;
	memcpy(p, ssid, slen);     p += slen;
	*p++ = plen;
	memcpy(p, password, plen); p += plen;
	ble_gattc_write_flat(s.conn_handle, s.cred_val_h,
	                     buf, (size_t)(p - buf), cred_write_cb, NULL);
	ESP_LOGI(TAG, "sending creds for ssid=%s", ssid);
}
