#include "theme.h"

const app_theme_t theme_dark = {
    .bg_primary    = 0x141E2B,
    .bg_secondary  = 0x0D1520,
    .text_primary  = 0xECEFF1,
    .text_muted    = 0x546E7A,
    .border        = 0x1E2D3D,
    .accent        = 0x2196F3,
    .accent_text   = 0xFFFFFF,
    .success       = 0x1B5E20,
    .success_text  = 0xA5D6A7,
    .warning       = 0x1A0F00,
    .warning_text  = 0xFFCC80,
    .error_bg      = 0x1A0A0A,
    .error_text    = 0xEF9A9A,
};

const app_theme_t theme_light = {
    .bg_primary    = 0xF5F7FA,
    .bg_secondary  = 0xFFFFFF,
    .text_primary  = 0x1A2332,
    .text_muted    = 0x78909C,
    .border        = 0xCFD8DC,
    .accent        = 0x1565C0,
    .accent_text   = 0xFFFFFF,
    .success       = 0xE8F5E9,
    .success_text  = 0x1B5E20,
    .warning       = 0xFFF8E1,
    .warning_text  = 0xE65100,
    .error_bg      = 0xFFEBEE,
    .error_text    = 0xB71C1C,
};
