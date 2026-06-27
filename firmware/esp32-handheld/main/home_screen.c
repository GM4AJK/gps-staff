#include "home_screen.h"
#include "logo_img.h"

static lv_obj_t       *s_panel;
static home_screen_cb_t s_cb;

static void base_btn_cb(lv_event_t *e)
{
	(void)e;
	if (s_cb) s_cb(true);
}

void home_screen_init(lv_obj_t *parent, home_screen_cb_t cb)
{
	s_cb = cb;

	s_panel = lv_obj_create(parent);
	lv_obj_set_size(s_panel, LV_PCT(100), LV_PCT(100));
	lv_obj_align(s_panel, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_set_style_bg_color(s_panel, lv_color_black(), 0);
	lv_obj_set_style_border_width(s_panel, 0, 0);
	lv_obj_set_style_pad_all(s_panel, 0, 0);
	lv_obj_clear_flag(s_panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(s_panel, LV_OBJ_FLAG_HIDDEN);

	/* Logo — 250×375, centred, 18px from top */
	lv_obj_t *img = lv_img_create(s_panel);
	lv_img_set_src(img, &logo_img);
	lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 18);

	/* Rover button — greyed out, no action yet */
	lv_obj_t *rover_btn = lv_btn_create(s_panel);
	lv_obj_set_size(rover_btn, 240, 65);
	lv_obj_align(rover_btn, LV_ALIGN_BOTTOM_LEFT, 80, -10);
	lv_obj_set_style_bg_color(rover_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
	lv_obj_t *rover_lbl = lv_label_create(rover_btn);
	lv_label_set_text(rover_lbl, "Rover");
	lv_obj_set_style_text_font(rover_lbl, &lv_font_montserrat_24, 0);
	lv_obj_center(rover_lbl);

	/* Base button */
	lv_obj_t *base_btn = lv_btn_create(s_panel);
	lv_obj_set_size(base_btn, 240, 65);
	lv_obj_align(base_btn, LV_ALIGN_BOTTOM_RIGHT, -80, -10);
	lv_obj_set_style_bg_color(base_btn, lv_palette_main(LV_PALETTE_BLUE), 0);
	lv_obj_t *base_lbl = lv_label_create(base_btn);
	lv_label_set_text(base_lbl, "Base");
	lv_obj_set_style_text_font(base_lbl, &lv_font_montserrat_24, 0);
	lv_obj_center(base_lbl);
	lv_obj_add_event_cb(base_btn, base_btn_cb, LV_EVENT_CLICKED, NULL);
}

void home_screen_show(void)
{
	lv_obj_clear_flag(s_panel, LV_OBJ_FLAG_HIDDEN);
}

void home_screen_hide(void)
{
	lv_obj_add_flag(s_panel, LV_OBJ_FLAG_HIDDEN);
}
