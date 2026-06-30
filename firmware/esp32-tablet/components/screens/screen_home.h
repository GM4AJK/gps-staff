#pragma once

#include "lvgl.h"
#include "theme.h"

// Create the Home Screen object (does not make it active).
// Call lv_scr_load() on the returned object to display it.
lv_obj_t *screen_home_create(const app_theme_t *t);
