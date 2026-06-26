#pragma once
#include "waveshare_rgb_lcd_port.h"
#include "ble_base_client.h"

typedef struct {
	lv_obj_t *list_panel;
	lv_obj_t *status;
	lv_obj_t *list;
	lv_obj_t *pwd_panel;
	lv_obj_t *pwd_ssid_label;
	lv_obj_t *pwd_ta;
	lv_obj_t *pwd_kb;
	prov_ap_t aps[20];
	uint8_t   ap_count;
	char      selected_ssid[33];
} wifi_provision_t;

void wifi_provision_init(lv_obj_t *parent, wifi_provision_t *w);
void wifi_provision_update(wifi_provision_t *w, const prov_ap_t *aps, uint8_t count);
