#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs.h"
#include "host/ble_hs.h"
#include "wifi_prov.h"

#define TAG               "wifi_prov"
#define PROV_SVC_UUID     0xAC00
#define PROV_CHR_UUID     0xAC01
#define CRED_CHR_UUID     0xAC02
#define RESULT_CHR_UUID   0xAC03
#define SCAN_INTERVAL_MS  3000
#define CONNECT_TIMEOUT_MS 15000
#define MAX_APS           20
#define NVS_NS            "wifi_prov"

typedef enum {
	PROV_STATE_SCANNING,
	PROV_STATE_CONNECTING,
	PROV_STATE_CONNECTED,
} prov_state_t;

static uint16_t          s_ap_list_handle;
static uint16_t          s_result_handle;
static volatile uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static wifi_ap_record_t  s_aps[MAX_APS];
static prov_state_t      s_state;
static SemaphoreHandle_t s_cred_sem;
static EventGroupHandle_t s_wifi_eg;
static char              s_pending_ssid[33];
static char              s_pending_pwd[65];
static uint8_t           s_fail_reason;
static uint32_t          s_got_ip;

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1
#define WIFI_DISC_BIT       BIT2

/* ---- NVS helpers ---- */

static bool nvs_load_creds(void)
{
	nvs_handle_t h;
	if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK)
		return false;
	size_t slen = sizeof(s_pending_ssid);
	size_t plen = sizeof(s_pending_pwd);
	bool ok = (nvs_get_str(h, "ssid", s_pending_ssid, &slen) == ESP_OK &&
	           nvs_get_str(h, "pwd",  s_pending_pwd,  &plen) == ESP_OK);
	nvs_close(h);
	return ok;
}

static void nvs_save_creds(void)
{
	nvs_handle_t h;
	if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK)
		return;
	nvs_set_str(h, "ssid", s_pending_ssid);
	nvs_set_str(h, "pwd",  s_pending_pwd);
	nvs_commit(h);
	nvs_close(h);
	ESP_LOGI(TAG, "credentials saved to NVS");
}

/* ---- WiFi event handler ---- */

static void wifi_event_handler(void *arg, esp_event_base_t base,
                                int32_t event_id, void *data)
{
	if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		wifi_event_sta_disconnected_t *e = (wifi_event_sta_disconnected_t *)data;
		s_fail_reason = e->reason;
		xEventGroupSetBits(s_wifi_eg, WIFI_FAIL_BIT | WIFI_DISC_BIT);
	} else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
		s_got_ip = e->ip_info.ip.addr;
		xEventGroupSetBits(s_wifi_eg, WIFI_CONNECTED_BIT);
	}
}

/* ---- WiFi reason → result code ---- */

static uint8_t map_reason(uint8_t r)
{
	switch (r) {
	case WIFI_REASON_AUTH_FAIL:
	case WIFI_REASON_AUTH_EXPIRE:
	case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
	case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
	case WIFI_REASON_IE_IN_4WAY_DIFFERS:
	case WIFI_REASON_HANDSHAKE_TIMEOUT:
		return 1; /* wrong password */
	case WIFI_REASON_NO_AP_FOUND:
		return 2; /* AP not found */
	default:
		return 4; /* other */
	}
}

/* ---- BLE notify helpers ---- */

static void notify_result(uint8_t status)
{
	uint16_t ch = s_conn_handle;
	if (ch == BLE_HS_CONN_HANDLE_NONE || !s_result_handle)
		return;

	/*
	 * 0xAC03 wire format (39 bytes):
	 *   [0]     status (0=connected, 1=wrong_pwd, 2=no_ap, 3=timeout, 4=error)
	 *   [1..33] SSID null-terminated (32 chars + null)
	 *   [34]    RSSI (int8_t as uint8_t, 0 if not connected)
	 *   [35..38] IP (uint32_t little-endian, 0 if not connected)
	 */
	uint8_t pkt[39] = {0};
	pkt[0] = status;
	strncpy((char *)pkt + 1, s_pending_ssid, 32);
	if (status == 0) {
		wifi_ap_record_t ap;
		if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK)
			pkt[34] = (uint8_t)ap.rssi;
		pkt[35] = (uint8_t)(s_got_ip);
		pkt[36] = (uint8_t)(s_got_ip >> 8);
		pkt[37] = (uint8_t)(s_got_ip >> 16);
		pkt[38] = (uint8_t)(s_got_ip >> 24);
	}

	struct os_mbuf *om = ble_hs_mbuf_from_flat(pkt, sizeof(pkt));
	if (!om)
		return;
	int rc = ble_gatts_notify_custom(ch, s_result_handle, om);
	if (rc != 0)
		ESP_LOGW(TAG, "result notify rc=%d", rc);
}

/* ---- GATT service definition ---- */

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
	memcpy(s_pending_ssid, p, slen);
	s_pending_ssid[slen] = '\0';
	p += slen;
	uint8_t plen = *p++;
	if ((p + plen) > (buf + len))
		return BLE_ATT_ERR_UNLIKELY;
	memcpy(s_pending_pwd, p, plen);
	s_pending_pwd[plen] = '\0';

	ESP_LOGI(TAG, "credentials received: ssid=%s", s_pending_ssid);
	xSemaphoreGive(s_cred_sem);
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
				.val_handle = &s_ap_list_handle,
				.access_cb  = prov_access_cb,
			},
			{
				.uuid      = BLE_UUID16_DECLARE(CRED_CHR_UUID),
				.flags     = BLE_GATT_CHR_F_WRITE,
				.access_cb = cred_access_cb,
			},
			{
				.uuid       = BLE_UUID16_DECLARE(RESULT_CHR_UUID),
				.flags      = BLE_GATT_CHR_F_NOTIFY,
				.val_handle = &s_result_handle,
				.access_cb  = prov_access_cb,
			},
			{ 0 }
		},
	},
	{ 0 }
};

/* ---- Scan helpers ---- */

static int rssi_cmp(const void *a, const void *b)
{
	return ((wifi_ap_record_t *)b)->rssi - ((wifi_ap_record_t *)a)->rssi;
}

static void notify_ap_list(uint16_t ch)
{
	if (ch == BLE_HS_CONN_HANDLE_NONE)
		return;

	/* worst-case: 1 count + MAX_APS × (1 ssid_len + 32 ssid + 1 rssi + 1 auth) */
	uint8_t pkt[1 + MAX_APS * 35];
	uint8_t *p = pkt;

	uint16_t ap_count = 0;
	esp_wifi_scan_start(NULL, true);
	esp_wifi_scan_get_ap_num(&ap_count);
	if (ap_count > MAX_APS)
		ap_count = MAX_APS;
	esp_wifi_scan_get_ap_records(&ap_count, s_aps);
	qsort(s_aps, ap_count, sizeof(wifi_ap_record_t), rssi_cmp);

	*p++ = (uint8_t)ap_count;
	for (int i = 0; i < ap_count; i++) {
		uint8_t slen = (uint8_t)strnlen((char *)s_aps[i].ssid, 32);
		*p++ = slen;
		memcpy(p, s_aps[i].ssid, slen);
		p += slen;
		*p++ = (uint8_t)s_aps[i].rssi;
		*p++ = (uint8_t)s_aps[i].authmode;
	}

	ESP_LOGI(TAG, "scan: %d APs", ap_count);
	struct os_mbuf *om = ble_hs_mbuf_from_flat(pkt, (size_t)(p - pkt));
	if (!om)
		return;
	int rc = ble_gatts_notify_custom(ch, s_ap_list_handle, om);
	if (rc != 0)
		ESP_LOGW(TAG, "ap_list notify rc=%d", rc);
}

/* ---- Main provisioning task ---- */

static void prov_task(void *arg)
{
	while (1) {
		switch (s_state) {

		case PROV_STATE_SCANNING:
			notify_ap_list(s_conn_handle);
			/* block up to 3 s waiting for credentials from cred_access_cb */
			if (xSemaphoreTake(s_cred_sem, pdMS_TO_TICKS(SCAN_INTERVAL_MS)) == pdTRUE)
				s_state = PROV_STATE_CONNECTING;
			break;

		case PROV_STATE_CONNECTING: {
			ESP_LOGI(TAG, "connecting to ssid=%s", s_pending_ssid);
			wifi_config_t wcfg = {0};
			strncpy((char *)wcfg.sta.ssid,     s_pending_ssid, 32);
			strncpy((char *)wcfg.sta.password,  s_pending_pwd,  64);
			esp_wifi_set_config(WIFI_IF_STA, &wcfg);

			xEventGroupClearBits(s_wifi_eg,
			                     WIFI_CONNECTED_BIT | WIFI_FAIL_BIT | WIFI_DISC_BIT);
			esp_wifi_connect();

			EventBits_t bits = xEventGroupWaitBits(s_wifi_eg,
			    WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdTRUE, pdFALSE,
			    pdMS_TO_TICKS(CONNECT_TIMEOUT_MS));

			if (bits & WIFI_CONNECTED_BIT) {
				ESP_LOGI(TAG, "connected to %s", s_pending_ssid);
				nvs_save_creds();
				s_state = PROV_STATE_CONNECTED;
				notify_result(0);
			} else {
				uint8_t code = (bits & WIFI_FAIL_BIT)
				               ? map_reason(s_fail_reason) : 3 /* timeout */;
				ESP_LOGW(TAG, "connect failed reason=%d code=%d",
				         s_fail_reason, code);
				s_state = PROV_STATE_SCANNING;
				notify_result(code);
			}
			break;
		}

		case PROV_STATE_CONNECTED:
			/* heartbeat every 3 s; wake immediately on unexpected WiFi drop */
			{
				EventBits_t bits = xEventGroupWaitBits(s_wifi_eg,
				    WIFI_DISC_BIT, pdTRUE, pdFALSE,
				    pdMS_TO_TICKS(SCAN_INTERVAL_MS));
				if (bits & WIFI_DISC_BIT) {
					ESP_LOGW(TAG, "WiFi dropped, returning to scan");
					s_state = PROV_STATE_SCANNING;
				} else {
					notify_result(0);
				}
			}
			break;
		}
	}
}

/* ---- Public API ---- */

void wifi_prov_register_svcs(void)
{
	ble_gatts_count_cfg(s_svcs);
	ble_gatts_add_svcs(s_svcs);
}

void wifi_prov_init(void)
{
	s_cred_sem = xSemaphoreCreateBinary();
	s_wifi_eg  = xEventGroupCreate();

	esp_netif_create_default_wifi_sta();

	esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
	esp_event_handler_register(IP_EVENT,   IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_start());

	if (nvs_load_creds()) {
		ESP_LOGI(TAG, "stored creds found, will connect to %s", s_pending_ssid);
		s_state = PROV_STATE_CONNECTING;
	} else {
		s_state = PROV_STATE_SCANNING;
	}
}

void wifi_prov_on_connect(uint16_t conn_handle)
{
	s_conn_handle = conn_handle;
}

void wifi_prov_on_disconnect(void)
{
	s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
}

void wifi_prov_start_task(void)
{
	xTaskCreate(prov_task, "wifi_prov", 8192, NULL, 5, NULL);
}
