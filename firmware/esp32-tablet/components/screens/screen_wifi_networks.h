#pragma once

#include "lvgl.h"
#include "theme.h"

lv_obj_t *screen_wifi_networks_create(const app_theme_t *t);
void screen_wifi_networks_set_nav_back(lv_obj_t *base_config_scr);
void screen_wifi_networks_set_nav_password(lv_obj_t *pwd_scr);
