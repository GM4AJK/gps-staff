#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_netif_ip_addr.h"
#include "lvgl_port.h"
#include "wifi_provision.h"
#include "ble_base_client.h"

#define TAG "wifi_prov_ui"

#define HEADER_H  56
#define CONTENT_H (LVGL_PORT_V_RES - HEADER_H)
#define KBD_H     300
#define PWD_KBD_CONTENT_H (LVGL_PORT_V_RES - KBD_H)

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
	/* Only switch panels if wifi_provision is currently active (at least one
	 * panel is visible).  Avoids BLE background callbacks revealing the UI
	 * while the user is on the home or base-config screen. */
	if (lv_obj_has_flag(w->list_panel, LV_OBJ_FLAG_HIDDEN) &&
	    lv_obj_has_flag(w->pwd_panel, LV_OBJ_FLAG_HIDDEN))
		return;
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

static void list_back_cb(lv_event_t *e)
{
	wifi_provision_t *w = (wifi_provision_t *)lv_event_get_user_data(e);
	wifi_provision_hide(w);
	if (w->back_cb) w->back_cb();
}

static void pwd_back_cb(lv_event_t *e)
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
	lv_obj_set_size(w->list_panel, LV_PCT(100), LV_PCT(100));
	lv_obj_align(w->list_panel, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(w->list_panel, lv_color_black(), 0);
	lv_obj_set_style_border_width(w->list_panel, 0, 0);
	lv_obj_set_style_pad_all(w->list_panel, 0, 0);
	lv_obj_clear_flag(w->list_panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(w->list_panel, LV_OBJ_FLAG_HIDDEN);

	/* Header row: Back button + status label */
	lv_obj_t *list_hdr = lv_obj_create(w->list_panel);
	lv_obj_set_size(list_hdr, LV_PCT(100), HEADER_H);
	lv_obj_align(list_hdr, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(list_hdr, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
	lv_obj_set_style_border_width(list_hdr, 0, 0);
	lv_obj_set_style_pad_all(list_hdr, 0, 0);
	lv_obj_clear_flag(list_hdr, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *list_back = lv_btn_create(list_hdr);
	lv_obj_align(list_back, LV_ALIGN_LEFT_MID, 8, 0);
	lv_obj_set_size(list_back, 90, 40);
	lv_obj_t *list_back_lbl = lv_label_create(list_back);
	lv_label_set_text(list_back_lbl, LV_SYMBOL_LEFT " Back");
	lv_obj_center(list_back_lbl);
	lv_obj_add_event_cb(list_back, list_back_cb, LV_EVENT_CLICKED, w);

	w->status = lv_label_create(list_hdr);
	lv_label_set_text(w->status, LV_SYMBOL_BLUETOOTH " Searching for GPS-Base...");
	lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_GREY), 0);
	lv_obj_align(w->status, LV_ALIGN_CENTER, 0, 0);

	w->list = lv_list_create(w->list_panel);
	lv_obj_set_pos(w->list, 0, HEADER_H);
	lv_obj_set_size(w->list, LV_PCT(100), CONTENT_H);
	lv_obj_set_style_bg_color(w->list, lv_color_black(), 0);
	lv_obj_set_style_border_width(w->list, 0, 0);
	lv_obj_set_style_radius(w->list, 0, 0);

	/* ---- Password panel (hidden) ---- */
	w->pwd_panel = lv_obj_create(parent);
	lv_obj_set_size(w->pwd_panel, LV_PCT(100), LV_PCT(100));
	lv_obj_align(w->pwd_panel, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(w->pwd_panel, lv_color_black(), 0);
	lv_obj_set_style_border_width(w->pwd_panel, 0, 0);
	lv_obj_set_style_pad_all(w->pwd_panel, 0, 0);
	lv_obj_clear_flag(w->pwd_panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(w->pwd_panel, LV_OBJ_FLAG_HIDDEN);

	/* Password panel header */
	lv_obj_t *pwd_hdr = lv_obj_create(w->pwd_panel);
	lv_obj_set_size(pwd_hdr, LV_PCT(100), HEADER_H);
	lv_obj_align(pwd_hdr, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(pwd_hdr, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
	lv_obj_set_style_border_width(pwd_hdr, 0, 0);
	lv_obj_set_style_pad_all(pwd_hdr, 0, 0);
	lv_obj_clear_flag(pwd_hdr, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *pwd_back = lv_btn_create(pwd_hdr);
	lv_obj_align(pwd_back, LV_ALIGN_LEFT_MID, 8, 0);
	lv_obj_set_size(pwd_back, 90, 40);
	lv_obj_t *pwd_back_lbl = lv_label_create(pwd_back);
	lv_label_set_text(pwd_back_lbl, LV_SYMBOL_LEFT " Back");
	lv_obj_center(pwd_back_lbl);
	lv_obj_add_event_cb(pwd_back, pwd_back_cb, LV_EVENT_CLICKED, w);

	w->pwd_ssid_label = lv_label_create(pwd_hdr);
	lv_label_set_text(w->pwd_ssid_label, "");
	lv_obj_set_style_text_color(w->pwd_ssid_label, lv_color_white(), 0);
	lv_obj_align(w->pwd_ssid_label, LV_ALIGN_CENTER, 0, 0);

	/* Password content area (above keyboard) */
	w->pwd_ta = lv_textarea_create(w->pwd_panel);
	lv_textarea_set_one_line(w->pwd_ta, true);
	lv_textarea_set_password_mode(w->pwd_ta, true);
	lv_textarea_set_placeholder_text(w->pwd_ta, "Password");
	lv_obj_set_width(w->pwd_ta, LV_PCT(90));
	lv_obj_align(w->pwd_ta, LV_ALIGN_TOP_MID, 0, HEADER_H + 16);

	lv_obj_t *conn_btn = lv_btn_create(w->pwd_panel);
	lv_obj_set_width(conn_btn, 200);
	lv_obj_align(conn_btn, LV_ALIGN_TOP_MID, 0, HEADER_H + 80);
	lv_obj_t *conn_lbl = lv_label_create(conn_btn);
	lv_label_set_text(conn_lbl, "Connect");
	lv_obj_center(conn_lbl);
	lv_obj_add_event_cb(conn_btn, connect_btn_cb, LV_EVENT_CLICKED, w);

	w->pwd_kb = lv_keyboard_create(w->pwd_panel);
	lv_keyboard_set_textarea(w->pwd_kb, w->pwd_ta);
	lv_obj_set_size(w->pwd_kb, LV_PCT(100), KBD_H);
	lv_obj_align(w->pwd_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
}

void wifi_provision_show(wifi_provision_t *w)
{
	/* Apply any cached connection result so the user sees the current Base
	 * state immediately without waiting for the next 3s heartbeat. */
	if (w->result_valid) {
		if (w->result_status == PROV_RESULT_OK) {
			char buf[80];
			esp_ip4_addr_t addr = { .addr = w->result_ip };
			snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Base: %s  %d dBm  " IPSTR,
			         w->result_ssid, w->result_rssi, IP2STR(&addr));
			lv_label_set_text(w->status, buf);
			lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_GREEN), 0);
			lv_obj_clean(w->list);
		}
	}
	lv_obj_clear_flag(w->list_panel, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(w->pwd_panel, LV_OBJ_FLAG_HIDDEN);
	lv_textarea_set_text(w->pwd_ta, "");
}

void wifi_provision_hide(wifi_provision_t *w)
{
	lv_obj_add_flag(w->list_panel, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(w->pwd_panel, LV_OBJ_FLAG_HIDDEN);
}

void wifi_provision_on_result(wifi_provision_t *w, uint8_t status, const char *ssid,
                               int8_t rssi, uint32_t ip)
{
	if (!lvgl_port_lock(-1))
		return;

	/* Cache the result so wifi_provision_show() can apply it immediately. */
	w->result_valid  = true;
	w->result_status = status;
	strncpy(w->result_ssid, ssid ? ssid : "", 32);
	w->result_ssid[32] = '\0';
	w->result_rssi   = rssi;
	w->result_ip     = ip;

	if (status == PROV_RESULT_OK) {
		char buf[80];
		esp_ip4_addr_t addr = { .addr = ip };
		snprintf(buf, sizeof(buf), LV_SYMBOL_OK " Base: %s  %d dBm  " IPSTR,
		         ssid, rssi, IP2STR(&addr));
		lv_label_set_text(w->status, buf);
		lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_GREEN), 0);
		lv_obj_clean(w->list);
	} else {
		const char *reason;
		switch (status) {
		case PROV_RESULT_WRONG_PWD: reason = "Wrong password";       break;
		case PROV_RESULT_NO_AP:     reason = "Network not found";    break;
		case PROV_RESULT_TIMEOUT:   reason = "Connection timed out"; break;
		default:                    reason = "Connection failed";     break;
		}
		lv_label_set_text(w->status, reason);
		lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_RED), 0);
	}
	/* always return to list panel so the user can see the result or retry */
	show_list(w);

	lvgl_port_unlock();
}

void wifi_provision_update(wifi_provision_t *w, const prov_ap_t *aps, uint8_t count)
{
	memcpy(w->aps, aps, count * sizeof(prov_ap_t));
	w->ap_count = count;

	if (!lvgl_port_lock(-1))
		return;

	lv_obj_clean(w->list);

	bool connected = w->result_valid && w->result_status == PROV_RESULT_OK;

	if (count == 0) {
		lv_label_set_text(w->status, LV_SYMBOL_WARNING " No WiFi networks found");
	} else {
		char buf[64];
		for (int i = 0; i < count; i++) {
			bool is_configured = connected &&
			                     strncmp(aps[i].ssid, w->result_ssid, 32) == 0;
			snprintf(buf, sizeof(buf), "%-20.20s  %4d dBm  %s",
			         aps[i].ssid, aps[i].rssi, auth_str(aps[i].auth));
			const char *icon = is_configured ? LV_SYMBOL_OK : LV_SYMBOL_WIFI;
			lv_obj_t *btn = lv_list_add_btn(w->list, icon, buf);
			lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
			lv_obj_set_style_text_color(btn,
			    is_configured ? lv_palette_main(LV_PALETTE_GREEN) : lv_color_white(), 0);
			lv_obj_set_user_data(btn, (void *)(uintptr_t)i);
			lv_obj_add_event_cb(btn, ap_btn_cb, LV_EVENT_CLICKED, w);
		}
		if (connected) {
			char buf2[80];
			esp_ip4_addr_t addr = { .addr = w->result_ip };
			snprintf(buf2, sizeof(buf2), LV_SYMBOL_OK " Base: %s  %d dBm  " IPSTR,
			         w->result_ssid, w->result_rssi, IP2STR(&addr));
			lv_label_set_text(w->status, buf2);
			lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_GREEN), 0);
		} else {
			snprintf(buf, sizeof(buf), LV_SYMBOL_WIFI " %d network%s",
			         count, count == 1 ? "" : "s");
			lv_label_set_text(w->status, buf);
			lv_obj_set_style_text_color(w->status, lv_palette_main(LV_PALETTE_GREY), 0);
		}
	}

	lvgl_port_unlock();
}
