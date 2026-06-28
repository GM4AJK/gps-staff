#include "home_screen.h"
#include "logo_img.h"

static void home_screen_base_btn_cb(lv_event_t *e)
{
	home_screen_t *h = (home_screen_t *)e->user_data;
	if (h->user_cb_base != NULL)
		h->user_cb_base(NULL);
}

static void home_screen_rover_btn_cb(lv_event_t *e)
{
	home_screen_t *h = (home_screen_t *)e->user_data;
	if (h->user_cb_rover != NULL)
		h->user_cb_rover(NULL);
}

void home_screen_init(lv_obj_t *parent, home_screen_t *h)
{
	h->displayed    = false;
	h->parent       = parent;
	h->cb_base      = home_screen_base_btn_cb;
	h->cb_rover     = home_screen_rover_btn_cb;
	h->user_cb_base = NULL;
	h->user_cb_rover = NULL;

	h->panel = lv_obj_create(parent);
	lv_obj_set_size(h->panel, LV_PCT(100), LV_PCT(100));
	lv_obj_align(h->panel, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(h->panel, lv_color_white(), 0);
	lv_obj_set_style_border_width(h->panel, 0, 0);
	lv_obj_set_style_pad_all(h->panel, 0, 0);
	lv_obj_clear_flag(h->panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(h->panel, LV_OBJ_FLAG_HIDDEN);

	h->img = lv_img_create(h->panel);
	lv_img_set_src(h->img, &logo_img);
	lv_obj_align(h->img, LV_ALIGN_CENTER, 0, 0);

	h->rover_btn = lv_btn_create(h->panel);
	lv_obj_set_size(h->rover_btn, 160, 60);
	lv_obj_align(h->rover_btn, LV_ALIGN_BOTTOM_MID, -100, -12);
	lv_obj_set_style_bg_color(h->rover_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_set_style_pad_all(h->rover_btn, 0, 0);
	h->rover_lbl = lv_label_create(h->rover_btn);
	lv_label_set_text(h->rover_lbl, "Rover");
	lv_obj_set_style_text_font(h->rover_lbl, &lv_font_montserrat_32, 0);
	lv_obj_center(h->rover_lbl);
	lv_obj_add_event_cb(h->rover_btn, home_screen_rover_btn_cb, LV_EVENT_CLICKED, h);

	h->base_btn = lv_btn_create(h->panel);
	lv_obj_set_size(h->base_btn, 160, 60);
	lv_obj_align(h->base_btn, LV_ALIGN_BOTTOM_MID, 100, -12);
	lv_obj_set_style_bg_color(h->base_btn, lv_palette_main(LV_PALETTE_BLUE), 0);
	lv_obj_set_style_pad_all(h->base_btn, 0, 0);
	h->base_lbl = lv_label_create(h->base_btn);
	lv_label_set_text(h->base_lbl, "Base");
	lv_obj_set_style_text_font(h->base_lbl, &lv_font_montserrat_32, 0);
	lv_obj_center(h->base_lbl);
	lv_obj_add_event_cb(h->base_btn, home_screen_base_btn_cb, LV_EVENT_CLICKED, h);
}

void home_screen_show(home_screen_t *h)
{
	h->displayed = true;
	lv_obj_clear_flag(h->panel, LV_OBJ_FLAG_HIDDEN);
}

void home_screen_hide(home_screen_t *h)
{
	h->displayed = false;
	lv_obj_add_flag(h->panel, LV_OBJ_FLAG_HIDDEN);
}
