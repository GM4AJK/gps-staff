#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "host/ble_hs.h"
#include "wifi_prov.h"

#define TAG              "wifi_prov"
#define PROV_SVC_UUID    0xAC00
#define PROV_CHR_UUID    0xAC01
#define CRED_CHR_UUID    0xAC02
#define SCAN_INTERVAL_MS 3000
#define MAX_APS          20

static uint16_t             s_val_handle;
static volatile uint16_t    s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static wifi_ap_record_t     s_aps[MAX_APS];
static wifi_prov_cred_cb_t  s_cred_cb;

static int prov_access_cb(uint16_t conn_h, uint16_t attr_h,
                           struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	return BLE_ATT_ERR_READ_NOT_PERMITTED;
}

static int cred_access_cb(uint16_t conn_h, uint16_t attr_h,
                           struct ble_gatt_access_ctxt *ctxt, void *arg)
{
	if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR)
		return BLE_ATT_ERR_UNLIKELY;

	uint8_t buf[1 + 32 + 1 + 64];
	uint16_t len = OS_MBUF_PKTLEN(ctxt->om);
	if (len > sizeof(buf))
		return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
	os_mbuf_copydata(ctxt->om, 0, len, buf);

	uint8_t *p = buf;
	uint8_t slen = *p++;
	if (slen > 32 || (p + slen + 1) > (buf + len))
		return BLE_ATT_ERR_UNLIKELY;
	char ssid[33];
	memcpy(ssid, p, slen);
	ssid[slen] = '\0';
	p += slen;
	uint8_t plen = *p++;
	if ((p + plen) > (buf + len))
		return BLE_ATT_ERR_UNLIKELY;
	char password[65];
	memcpy(password, p, plen);
	password[plen] = '\0';

	ESP_LOGI(TAG, "credentials received: ssid=%s", ssid);
	if (s_cred_cb)
		s_cred_cb(ssid, password);

	return 0;
}

static const struct ble_gatt_svc_def s_svcs[] = {
	{
		.type = BLE_GATT_SVC_TYPE_PRIMARY,
		.uuid = BLE_UUID16_DECLARE(PROV_SVC_UUID),
		.characteristics = (struct ble_gatt_chr_def[]) {
			{
				.uuid       = BLE_UUID16_DECLARE(PROV_CHR_UUID),
				.flags      = BLE_GATT_CHR_F_NOTIFY,
				.val_handle = &s_val_handle,
				.access_cb  = prov_access_cb,
			},
			{
				.uuid      = BLE_UUID16_DECLARE(CRED_CHR_UUID),
				.flags     = BLE_GATT_CHR_F_WRITE,
				.access_cb = cred_access_cb,
			},
			{ 0 }
		},
	},
	{ 0 }
};

static int rssi_cmp(const void *a, const void *b)
{
	return ((wifi_ap_record_t *)b)->rssi - ((wifi_ap_record_t *)a)->rssi;
}

void wifi_prov_register_svcs(void)
{
	ble_gatts_count_cfg(s_svcs);
	ble_gatts_add_svcs(s_svcs);
}

void wifi_prov_init(void)
{
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void wifi_prov_on_connect(uint16_t conn_handle)
{
	s_conn_handle = conn_handle;
}

void wifi_prov_on_disconnect(void)
{
	s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
}

static void scan_task(void *arg)
{
	/* worst-case: 1 byte count + MAX_APS × (1 ssid_len + 32 ssid + 1 rssi + 1 auth) */
	uint8_t  pkt[1 + MAX_APS * 35];
	TickType_t wake = xTaskGetTickCount();

	while (1) {
		vTaskDelayUntil(&wake, pdMS_TO_TICKS(SCAN_INTERVAL_MS));

		uint16_t ap_count = 0;
		esp_wifi_scan_start(NULL, true);
		esp_wifi_scan_get_ap_num(&ap_count);
		if (ap_count > MAX_APS)
			ap_count = MAX_APS;
		esp_wifi_scan_get_ap_records(&ap_count, s_aps);
		qsort(s_aps, ap_count, sizeof(wifi_ap_record_t), rssi_cmp);

		uint8_t *p = pkt;
		*p++ = (uint8_t)ap_count;
		for (int i = 0; i < ap_count; i++) {
			uint8_t slen = (uint8_t)strnlen((char *)s_aps[i].ssid, 32);
			*p++ = slen;
			memcpy(p, s_aps[i].ssid, slen);
			p += slen;
			*p++ = (uint8_t)s_aps[i].rssi;
			*p++ = (uint8_t)s_aps[i].authmode;
		}
		size_t pkt_len = p - pkt;

		ESP_LOGI(TAG, "scan: %d APs", ap_count);

		uint16_t ch = s_conn_handle;
		if (ch == BLE_HS_CONN_HANDLE_NONE)
			continue;

		struct os_mbuf *om = ble_hs_mbuf_from_flat(pkt, pkt_len);
		if (!om)
			continue;
		int rc = ble_gatts_notify_custom(ch, s_val_handle, om);
		if (rc != 0)
			ESP_LOGW(TAG, "notify rc=%d", rc);
		else
			ESP_LOGI(TAG, "notified %d APs (%zu bytes)", ap_count, pkt_len);
	}
}

void wifi_prov_set_cred_cb(wifi_prov_cred_cb_t cb)
{
	s_cred_cb = cb;
}

void wifi_prov_start_task(void)
{
	xTaskCreate(scan_task, "wifi_prov", 8192, NULL, 5, NULL);
}
