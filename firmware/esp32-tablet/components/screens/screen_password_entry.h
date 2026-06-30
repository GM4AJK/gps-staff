#pragma once

#include "lvgl.h"
#include "theme.h"

lv_obj_t *screen_password_entry_create(const app_theme_t *t);
void screen_password_entry_set_nav_back(lv_obj_t *wifi_scr);
