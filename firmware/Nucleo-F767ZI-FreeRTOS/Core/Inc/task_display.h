#pragma once

#include <stdint.h>
#include <stdarg.h>
#include "ssd1309.h"

void display_init(void);
void display_clear(void);
void display_string(int16_t x, int16_t y, const ssd1309_font_t *font, const char *fmt, ...);
void display_flush(void);
