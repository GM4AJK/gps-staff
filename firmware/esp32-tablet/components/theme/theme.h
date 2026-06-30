#pragma once
#include <stdint.h>
#include "lvgl.h"

// Color palette as plain RGB888 hex constants so the struct can be a compile-time
// literal. Convert to lv_color_t at the call site with lv_color_hex(t->field).
typedef struct {
    uint32_t bg_primary;
    uint32_t bg_secondary;
    uint32_t text_primary;
    uint32_t text_muted;
    uint32_t border;
    uint32_t accent;
    uint32_t accent_text;
    uint32_t success;
    uint32_t success_text;
    uint32_t warning;
    uint32_t warning_text;
    uint32_t error_bg;
    uint32_t error_text;
} app_theme_t;

// Pass `const app_theme_t *t` to every screen_xxx_create() function.
// Switching a screen's theme is a one-word change at the call site.
extern const app_theme_t theme_dark;
extern const app_theme_t theme_light;

// Convenience: convert a theme colour field to lv_color_t inline.
static inline lv_color_t tc(uint32_t hex) { return lv_color_hex(hex); }
