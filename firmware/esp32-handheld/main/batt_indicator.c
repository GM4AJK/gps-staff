
#include "batt_indicator.h"


static void batt_indicator_charging_anim_cb(lv_timer_t *timer)
{
    batt_indicator_t *b = (batt_indicator_t *)timer->user_data;
    b->charge_pct += 10;
    if (b->charge_pct > 100)
        b->charge_pct = 10;
    lv_obj_set_width(b->batt_fill, lv_pct(b->charge_pct));
}

void batt_indicator_init(lv_obj_t *parent, batt_indicator_t *b)
{
    b->parent = parent;
    b->charge_pct = 10;
    b->body = lv_obj_create(parent);
    lv_obj_set_size(b->body, 60, 26);
    lv_obj_align(b->body, LV_ALIGN_TOP_RIGHT, -18, 10);
    lv_obj_set_scrollbar_mode(b->body, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(b->body, lv_color_black(), 0);
    lv_obj_set_style_border_color(b->body, lv_color_white(), 0);
    lv_obj_set_style_border_width(b->body, 2, 0);
    lv_obj_set_style_radius(b->body, 3, 0);
    lv_obj_set_style_pad_all(b->body, 3, 0);

    b->batt_fill = lv_obj_create(b->body);
    lv_obj_set_height(b->batt_fill, lv_pct(100));
    lv_obj_set_width(b->batt_fill, lv_pct(b->charge_pct));
    lv_obj_align(b->batt_fill, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_scrollbar_mode(b->batt_fill, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(b->batt_fill, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_style_border_width(b->batt_fill, 0, 0);
    lv_obj_set_style_radius(b->batt_fill, 1, 0);

    b->nub = lv_obj_create(parent);
    lv_obj_set_size(b->nub, 5, 12);
    lv_obj_align(b->nub, LV_ALIGN_TOP_RIGHT, -12, 17);
    lv_obj_set_style_bg_color(b->nub, lv_color_white(), 0);
    lv_obj_set_style_border_width(b->nub, 0, 0);
    lv_obj_set_style_radius(b->nub, 2, 0);

    /* Lightning bolt — centred over body, slightly taller */
    b->bolt = lv_label_create(parent);
    lv_label_set_text(b->bolt, LV_SYMBOL_CHARGE);
    lv_obj_set_style_text_font(b->bolt, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(b->bolt, lv_palette_main(LV_PALETTE_YELLOW), 0);
    lv_obj_align_to(b->bolt, b->body, LV_ALIGN_CENTER, 0, 0);

    lv_timer_create(batt_indicator_charging_anim_cb, 600, b);    
}
