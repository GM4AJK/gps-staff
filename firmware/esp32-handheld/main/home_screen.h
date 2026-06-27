#pragma once
#include "waveshare_rgb_lcd_port.h"

typedef void (*home_screen_cb_t)(bool is_base);

void home_screen_init(lv_obj_t *parent, home_screen_cb_t cb);
void home_screen_show(void);
void home_screen_hide(void);
