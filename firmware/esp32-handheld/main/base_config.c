#include "base_config.h"
#include "lvgl_port.h"

static lv_obj_t           *s_panel;
static base_config_back_cb_t s_back_cb;
static wifi_provision_t    *s_prov;

static void on_wifi_back(void)
{
	base_config_show();
}

static void back_btn_cb(lv_event_t *e)
{
	(void)e;
	base_config_hide();
	if (s_back_cb) s_back_cb();
}

static void wifi_item_cb(lv_event_t *e)
{
	(void)e;
	base_config_hide();
	wifi_provision_show(s_prov);
}

void base_config_init(lv_obj_t *parent, wifi_provision_t *prov,
                      base_config_back_cb_t back_cb)
{
	s_prov    = prov;
	s_back_cb = back_cb;
	prov->back_cb = on_wifi_back;

	s_panel = lv_obj_create(parent);
	lv_obj_set_size(s_panel, LV_PCT(100), LV_PCT(100));
	lv_obj_align(s_panel, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(s_panel, lv_color_black(), 0);
	lv_obj_set_style_border_width(s_panel, 0, 0);
	lv_obj_set_style_pad_all(s_panel, 0, 0);
	lv_obj_clear_flag(s_panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(s_panel, LV_OBJ_FLAG_HIDDEN);

	/* Header bar */
	lv_obj_t *header = lv_obj_create(s_panel);
	lv_obj_set_size(header, LV_PCT(100), 56);
	lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(header, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
	lv_obj_set_style_border_width(header, 0, 0);
	lv_obj_set_style_pad_all(header, 0, 0);
	lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *back_btn = lv_btn_create(header);
	lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 8, 0);
	lv_obj_set_size(back_btn, 90, 40);
	lv_obj_t *back_lbl = lv_label_create(back_btn);
	lv_label_set_text(back_lbl, LV_SYMBOL_LEFT " Back");
	lv_obj_center(back_lbl);
	lv_obj_add_event_cb(back_btn, back_btn_cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t *title = lv_label_create(header);
	lv_label_set_text(title, "Base Configuration");
	lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
	lv_obj_set_style_text_color(title, lv_color_white(), 0);
	lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

	/* Menu list */
	lv_obj_t *menu = lv_list_create(s_panel);
	lv_obj_set_pos(menu, 0, 56);
	lv_obj_set_size(menu, LV_PCT(100), LVGL_PORT_V_RES - 56);
	lv_obj_set_style_bg_color(menu, lv_color_black(), 0);
	lv_obj_set_style_border_width(menu, 0, 0);
	lv_obj_set_style_radius(menu, 0, 0);

	lv_obj_t *wifi_btn = lv_list_add_btn(menu, LV_SYMBOL_WIFI, "WiFi Settings");
	lv_obj_set_style_bg_color(wifi_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_set_style_text_color(wifi_btn, lv_color_white(), 0);
	lv_obj_add_event_cb(wifi_btn, wifi_item_cb, LV_EVENT_CLICKED, NULL);

	/* Placeholder items — greyed, no event */
	lv_obj_t *survey_btn = lv_list_add_btn(menu, LV_SYMBOL_GPS, "Survey-in Settings");
	lv_obj_set_style_bg_color(survey_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_set_style_text_color(survey_btn, lv_palette_main(LV_PALETTE_GREY), 0);

	lv_obj_t *fix_btn = lv_list_add_btn(menu, LV_SYMBOL_GPS, "Fix Selection");
	lv_obj_set_style_bg_color(fix_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_set_style_text_color(fix_btn, lv_palette_main(LV_PALETTE_GREY), 0);
}

void base_config_show(void)
{
	lv_obj_clear_flag(s_panel, LV_OBJ_FLAG_HIDDEN);
}

void base_config_hide(void)
{
	lv_obj_add_flag(s_panel, LV_OBJ_FLAG_HIDDEN);
}
