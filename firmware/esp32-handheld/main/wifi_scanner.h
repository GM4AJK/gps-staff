/*
 * Author: Andy Kirkham
 */
#pragma once

#include "waveshare_rgb_lcd_port.h"

typedef struct
{
	lv_obj_t *status;
	lv_obj_t *list;
} wifi_scanner_t;

void wifi_scanner_init(lv_obj_t *parent, wifi_scanner_t *w);
