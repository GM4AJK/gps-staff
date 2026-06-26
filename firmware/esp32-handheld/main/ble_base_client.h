#pragma once
#include <stdint.h>
#include <stddef.h>

/* WiFi auth mode values — mirrors wifi_auth_mode_t wire encoding from Base */
#define PROV_AUTH_OPEN     0
#define PROV_AUTH_WEP      1
#define PROV_AUTH_WPA      2
#define PROV_AUTH_WPA2     3
#define PROV_AUTH_WPA_WPA2 4
#define PROV_AUTH_ENT      5
#define PROV_AUTH_WPA3     6
#define PROV_AUTH_WPA2_3   7

typedef struct {
	char    ssid[33];
	int8_t  rssi;
	uint8_t auth;
} prov_ap_t;

typedef void (*prov_ap_list_cb_t)(const prov_ap_t *aps, uint8_t count);

void ble_base_client_init(prov_ap_list_cb_t cb);
void ble_base_client_on_sync(void);
void ble_base_client_send_creds(const char *ssid, const char *password);
