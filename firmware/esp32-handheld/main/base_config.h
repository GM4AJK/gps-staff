#pragma once
#include "waveshare_rgb_lcd_port.h"
#include "wifi_provision.h"

typedef void (*base_config_back_cb_t)(void);

void base_config_init(lv_obj_t *parent, wifi_provision_t *prov,
                      base_config_back_cb_t back_cb);
void base_config_show(void);
void base_config_hide(void);
