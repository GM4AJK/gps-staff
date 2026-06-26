#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "lvgl_port.h"
#include "wifi_provision.h"
#include "ble_base_client.h"

#define TAG "wifi_prov_ui"

static const char *auth_str(uint8_t auth)
{
	switch (auth) {
	case PROV_AUTH_OPEN:     return "Open";
	case PROV_AUTH_WEP:      return "WEP";
	case PROV_AUTH_WPA:      return "WPA";
	case PROV_AUTH_WPA2:     return "WPA2";
	case PROV_AUTH_WPA_WPA2: return "WPA/2";
	case PROV_AUTH_WPA3:     return "WPA3";
	case PROV_AUTH_WPA2_3:   return "WPA2/3";
	default:                 return "Ent";
	}
}

static void show_list(wifi_provision_t *w)
{
	lv_obj_clear_flag(w->list_panel, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(w->pwd_panel, LV_OBJ_FLAG_HIDDEN);
	lv_textarea_set_text(w->pwd_ta, "");
}

static void show_pwd(wifi_provision_t *w, const char *ssid)
{
	strncpy(w->selected_ssid, ssid, 32);
	w->selected_ssid[32] = '\0';

	char buf[48];
	snprintf(buf, sizeof(buf), "Connect to: %s", ssid);
	lv_label_set_text(w->pwd_ssid_label, buf);
	lv_textarea_set_text(w->pwd_ta, "");

	lv_obj_add_flag(w->list_panel, LV_OBJ_FLAG_HIDDEN);
	lv_obj_clear_flag(w->pwd_panel, LV_OBJ_FLAG_HIDDEN);
}

static void back_btn_cb(lv_event_t *e)
{
	show_list((wifi_provision_t *)lv_event_get_user_data(e));
}

static void connect_btn_cb(lv_event_t *e)
{
	wifi_provision_t *w = (wifi_provision_t *)lv_event_get_user_data(e);
	const char *pwd = lv_textarea_get_text(w->pwd_ta);
	ESP_LOGI(TAG, "sending creds ssid=%s", w->selected_ssid);
	ble_base_client_send_creds(w->selected_ssid, pwd);
	lv_label_set_text(w->pwd_ssid_label, "Sending to Base...");
}

static void ap_btn_cb(lv_event_t *e)
{
	wifi_provision_t *w   = (wifi_provision_t *)lv_event_get_user_data(e);
	lv_obj_t         *btn = lv_event_get_target(e);
	int idx = (int)(uintptr_t)lv_obj_get_user_data(btn);
	if (idx >= w->ap_count)
		return;

	if (w->aps[idx].auth == PROV_AUTH_OPEN) {
		ble_base_client_send_creds(w->aps[idx].ssid, "");
		return;
	}
	show_pwd(w, w->aps[idx].ssid);
}

void wifi_provision_init(lv_obj_t *parent, wifi_provision_t *w)
{
	/* ---- List panel ---- */
	w->list_panel = lv_obj_create(parent);
	lv_obj_set_size(w->list_panel, LV_PCT(100), 790);
	lv_obj_align(w->list_panel, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_bg_color(w->list_panel, lv_color_black(), 0);
	lv_obj_set_style_border_width(w->list_panel, 0, 0);
	lv_obj_set_style_pad_all(w->list_panel, 0, 0);

	w->status = lv_label_create(w->list_panel);
	lv_label_set_text(w->status, LV_SYMBOL_BLUETOOTH " Searching for GPS-Base...");
	lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_GREY), 0);
	lv_obj_align(w->status, LV_ALIGN_TOP_MID, 0, 8);

	w->list = lv_list_create(w->list_panel);
	lv_obj_set_pos(w->list, 0, 46);
	lv_obj_set_size(w->list, LV_PCT(100), 744);
	lv_obj_set_style_bg_color(w->list, lv_color_black(), 0);
	lv_obj_set_style_border_width(w->list, 0, 0);
	lv_obj_set_style_radius(w->list, 0, 0);

	/* ---- Password panel (hidden) ---- */
	w->pwd_panel = lv_obj_create(parent);
	lv_obj_set_size(w->pwd_panel, LV_PCT(100), 790);
	lv_obj_align(w->pwd_panel, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_bg_color(w->pwd_panel, lv_color_black(), 0);
	lv_obj_set_style_border_width(w->pwd_panel, 0, 0);
	lv_obj_set_style_pad_all(w->pwd_panel, 0, 0);
	lv_obj_add_flag(w->pwd_panel, LV_OBJ_FLAG_HIDDEN);

	lv_obj_t *back_btn = lv_btn_create(w->pwd_panel);
	lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 8, 8);
	lv_obj_t *back_lbl = lv_label_create(back_btn);
	lv_label_set_text(back_lbl, LV_SYMBOL_LEFT " Back");
	lv_obj_add_event_cb(back_btn, back_btn_cb, LV_EVENT_CLICKED, w);

	w->pwd_ssid_label = lv_label_create(w->pwd_panel);
	lv_label_set_text(w->pwd_ssid_label, "");
	lv_obj_set_style_text_color(w->pwd_ssid_label, lv_color_white(), 0);
	lv_obj_align(w->pwd_ssid_label, LV_ALIGN_TOP_MID, 0, 65);

	w->pwd_ta = lv_textarea_create(w->pwd_panel);
	lv_textarea_set_one_line(w->pwd_ta, true);
	lv_textarea_set_password_mode(w->pwd_ta, true);
	lv_textarea_set_placeholder_text(w->pwd_ta, "Password");
	lv_obj_set_width(w->pwd_ta, LV_PCT(90));
	lv_obj_align(w->pwd_ta, LV_ALIGN_TOP_MID, 0, 110);

	lv_obj_t *conn_btn = lv_btn_create(w->pwd_panel);
	lv_obj_set_width(conn_btn, 200);
	lv_obj_align(conn_btn, LV_ALIGN_TOP_MID, 0, 180);
	lv_obj_t *conn_lbl = lv_label_create(conn_btn);
	lv_label_set_text(conn_lbl, "Connect");
	lv_obj_center(conn_lbl);
	lv_obj_add_event_cb(conn_btn, connect_btn_cb, LV_EVENT_CLICKED, w);

	w->pwd_kb = lv_keyboard_create(w->pwd_panel);
	lv_keyboard_set_textarea(w->pwd_kb, w->pwd_ta);
	lv_obj_set_size(w->pwd_kb, LV_PCT(100), 300);
	lv_obj_align(w->pwd_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
}

void wifi_provision_update(wifi_provision_t *w, const prov_ap_t *aps, uint8_t count)
{
	memcpy(w->aps, aps, count * sizeof(prov_ap_t));
	w->ap_count = count;

	if (!lvgl_port_lock(-1))
		return;

	lv_obj_clean(w->list);

	if (count == 0) {
		lv_label_set_text(w->status, LV_SYMBOL_WARNING " No WiFi networks found");
	} else {
		char buf[64];
		for (int i = 0; i < count; i++) {
			snprintf(buf, sizeof(buf), "%-20.20s  %4d dBm  %s",
			         aps[i].ssid, aps[i].rssi, auth_str(aps[i].auth));
			lv_obj_t *btn = lv_list_add_btn(w->list, LV_SYMBOL_WIFI, buf);
			lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
			lv_obj_set_style_text_color(btn, lv_color_white(), 0);
			lv_obj_set_user_data(btn, (void *)(uintptr_t)i);
			lv_obj_add_event_cb(btn, ap_btn_cb, LV_EVENT_CLICKED, w);
		}
		snprintf(buf, sizeof(buf), LV_SYMBOL_WIFI " %d network%s",
		         count, count == 1 ? "" : "s");
		lv_label_set_text(w->status, buf);
	}

	lvgl_port_unlock();
}
