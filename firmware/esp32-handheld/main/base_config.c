#include "base_config.h"
#include "lvgl_port.h"

static void base_config_back_btn_cb(lv_event_t *e)
{
	base_config_t *b = (base_config_t *)e->user_data;
	if (b->user_cb_back != NULL)
		b->user_cb_back(NULL);
}

static void base_config_wifi_btn_cb(lv_event_t *e)
{
	base_config_t *b = (base_config_t *)e->user_data;
	if (b->user_cb_wifi != NULL)
		b->user_cb_wifi(NULL);
}

void base_config_init(lv_obj_t *parent, base_config_t *b)
{
	b->displayed     = false;
	b->parent        = parent;
	b->user_cb_back  = NULL;
	b->user_cb_wifi  = NULL;

	b->panel = lv_obj_create(parent);
	lv_obj_set_size(b->panel, LV_PCT(100), LV_PCT(100));
	lv_obj_align(b->panel, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(b->panel, lv_color_black(), 0);
	lv_obj_set_style_border_width(b->panel, 0, 0);
	lv_obj_set_style_pad_all(b->panel, 0, 0);
	lv_obj_clear_flag(b->panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(b->panel, LV_OBJ_FLAG_HIDDEN);

	b->header = lv_obj_create(b->panel);
	lv_obj_set_size(b->header, LV_PCT(100), 56);
	lv_obj_align(b->header, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(b->header, lv_palette_darken(LV_PALETTE_GREY, 4), 0);
	lv_obj_set_style_border_width(b->header, 0, 0);
	lv_obj_set_style_pad_all(b->header, 0, 0);
	lv_obj_clear_flag(b->header, LV_OBJ_FLAG_SCROLLABLE);

	b->back_btn = lv_btn_create(b->header);
	lv_obj_align(b->back_btn, LV_ALIGN_LEFT_MID, 8, 0);
	lv_obj_set_size(b->back_btn, 90, 40);
	lv_obj_t *back_lbl = lv_label_create(b->back_btn);
	lv_label_set_text(back_lbl, LV_SYMBOL_LEFT " Back");
	lv_obj_center(back_lbl);
	lv_obj_add_event_cb(b->back_btn, base_config_back_btn_cb, LV_EVENT_CLICKED, b);

	b->title = lv_label_create(b->header);
	lv_label_set_text(b->title, "Base Configuration");
	lv_obj_set_style_text_font(b->title, &lv_font_montserrat_24, 0);
	lv_obj_set_style_text_color(b->title, lv_color_white(), 0);
	lv_obj_align(b->title, LV_ALIGN_CENTER, 0, 0);

	b->menu = lv_list_create(b->panel);
	lv_obj_set_pos(b->menu, 0, 56);
	lv_obj_set_size(b->menu, LV_PCT(100), LVGL_PORT_V_RES - 56);
	lv_obj_set_style_bg_color(b->menu, lv_color_black(), 0);
	lv_obj_set_style_border_width(b->menu, 0, 0);
	lv_obj_set_style_radius(b->menu, 0, 0);

	b->wifi_btn = lv_list_add_btn(b->menu, LV_SYMBOL_WIFI, "WiFi Settings");
	lv_obj_set_style_bg_color(b->wifi_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_set_style_text_color(b->wifi_btn, lv_color_white(), 0);
	lv_obj_add_event_cb(b->wifi_btn, base_config_wifi_btn_cb, LV_EVENT_CLICKED, b);

	b->survey_btn = lv_list_add_btn(b->menu, LV_SYMBOL_GPS, "Survey-in Settings");
	lv_obj_set_style_bg_color(b->survey_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_set_style_text_color(b->survey_btn, lv_palette_main(LV_PALETTE_GREY), 0);

	b->fix_btn = lv_list_add_btn(b->menu, LV_SYMBOL_GPS, "Fix Selection");
	lv_obj_set_style_bg_color(b->fix_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_set_style_text_color(b->fix_btn, lv_palette_main(LV_PALETTE_GREY), 0);
}

void base_config_show(base_config_t *b)
{
	b->displayed = true;
	lv_obj_clear_flag(b->panel, LV_OBJ_FLAG_HIDDEN);
}

void base_config_hide(base_config_t *b)
{
	b->displayed = false;
	lv_obj_add_flag(b->panel, LV_OBJ_FLAG_HIDDEN);
}
