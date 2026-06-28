#pragma once
#include <stdbool.h>
#include "waveshare_rgb_lcd_port.h"


typedef struct
{
    lv_obj_t *parent;
    lv_obj_t *panel;
    lv_obj_t *img;
    lv_obj_t *rover_btn;
    lv_obj_t *rover_lbl;
    lv_obj_t *base_btn;
    lv_obj_t *base_lbl;
    void *user_data;
    bool displayed;
    void (*cb_base)(lv_event_t *e);
    void (*cb_rover)(lv_event_t *e);
    void (*user_cb_base)(void *e);
    void (*user_cb_rover)(void *e);
}
home_screen_t;

void home_screen_init(lv_obj_t *parent, home_screen_t *p);
void home_screen_show(home_screen_t *p);
void home_screen_hide(home_screen_t *p);
