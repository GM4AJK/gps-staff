#pragma once

#include <stdint.h>
#include <stdarg.h>
#include "ssd130x.h"

/**
 * display_init
 *
 * Creates the display message queue and spawns the display task, which owns
 * the SSD1309 and hi2c1. Must be called once before any other display_*
 * function, while the scheduler is running.
 */
void display_init(void);

/**
 * display_clear
 *
 * Queues a clear of the framebuffer (all pixels off). Takes effect when the
 * display task processes the message; does not flush to the panel.
 */
void display_clear(void);

/**
 * display_string
 * @param x   - Column of the first glyph's top-left pixel
 * @param y   - Row of the first glyph's top-left pixel
 * @param font - Font to draw with (e.g. &font5x7, &font8x8)
 * @param str  - NUL-terminated string to draw; truncated to DISPLAY_TEXT_LEN-1
 *
 * Queues a string draw into the framebuffer. str is copied into the message
 * on the calling task's stack — no va_list, safe for tasks with small stacks.
 * Call display_flush() to push the framebuffer to the panel. For formatted
 * output, snprintf into a local buffer first then pass the result here.
 */
void display_string(int16_t x, int16_t y, const ssd130x_font_t *font, const char *str);

/**
 * display_va_string
 * @param x    - Column of the first glyph's top-left pixel
 * @param y    - Row of the first glyph's top-left pixel
 * @param font - Font to draw with (e.g. &font5x7, &font8x8)
 * @param fmt  - printf-style format string
 * @param ...  - Format arguments
 *
 * Variadic form of display_string(). Calls vsnprintf() on the calling task's
 * stack — convenient for debug output but carries additional stack cost.
 * Prefer display_string() with a caller-side snprintf() in production tasks.
 */
void display_va_string(int16_t x, int16_t y, const ssd130x_font_t *font, const char *fmt, ...);

/**
 * display_flush
 *
 * Queues a flush of the framebuffer to the SSD1309 panel over I2C. The
 * display task holds hi2c1_mutex for the duration of the transfer; other
 * I2C users will block until it completes.
 */
void display_flush(void);
