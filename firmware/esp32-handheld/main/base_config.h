#pragma once
#include <stdbool.h>
#include "waveshare_rgb_lcd_port.h"

typedef struct {
	lv_obj_t *parent;
	lv_obj_t *panel;
	lv_obj_t *header;
	lv_obj_t *back_btn;
	lv_obj_t *title;
	lv_obj_t *menu;
	lv_obj_t *wifi_btn;
	lv_obj_t *survey_btn;
	lv_obj_t *fix_btn;
	bool displayed;
	void (*user_cb_back)(void *);
	void (*user_cb_wifi)(void *);
} base_config_t;

void base_config_init(lv_obj_t *parent, base_config_t *b);
void base_config_show(base_config_t *b);
void base_config_hide(base_config_t *b);
