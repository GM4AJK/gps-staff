#pragma once

#include "lvgl.h"
#include "theme.h"

// Create the Base Configuration screen (does not make it active).
lv_obj_t *screen_base_config_create(const app_theme_t *t);

// Register the screen to load when the Back button is pressed.
void screen_base_config_set_nav_back(lv_obj_t *home_scr);

// Register the screen to load when the WiFi Settings button is tapped.
void screen_base_config_set_nav_wifi(lv_obj_t *wifi_scr);
