#pragma once

#include <stdint.h>
#include "ssd1309.h"

#define DISPLAY_TEXT_LEN 64

typedef enum {
	DISPLAY_CLEAR,
	DISPLAY_STRING,
	DISPLAY_FLUSH,
} display_msg_type_t;

typedef struct {
	display_msg_type_t type;
	union {
		struct {
			int16_t x;
			int16_t y;
			const ssd1309_font_t *font;
			char text[DISPLAY_TEXT_LEN];
		} string;
	};
} display_msg_t;

void display_init(void);
void display_send(const display_msg_t *msg);
