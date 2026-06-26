#pragma once

#include "waveshare_rgb_lcd_port.h"

typedef struct 
{
    lv_obj_t *parent;
    lv_obj_t *batt_fill;
    lv_obj_t *body;
    lv_obj_t *nub;
    lv_obj_t *bolt;
    int charge_pct;
} 
batt_indicator_t;

void batt_indicator_init(lv_obj_t *parent, batt_indicator_t *b);

