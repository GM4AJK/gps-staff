#include "screen_password_entry.h"
#include "esp_log.h"

static const char *TAG = "screen_pwd";

static lv_obj_t *s_nav_back  = NULL;
static lv_obj_t *s_keyboard  = NULL;
static lv_obj_t *s_pwd_ta    = NULL;
static lv_obj_t *s_show_lbl  = NULL;

void screen_password_entry_set_nav_back(lv_obj_t *wifi_scr)
{
	s_nav_back = wifi_scr;
}

// ── Layout constants ──────────────────────────────────────────────────────────

#define STATUS_BAR_H  28
#define HEADER_BAR_H  52

// ── Colours ───────────────────────────────────────────────────────────────────

#define COL_STATUS_BG  0x111111
#define COL_HEADER_BG  0x222222
#define COL_WHITE      0xFFFFFF
#define COL_LEFT_BG    0xF5F5F5
#define COL_PANEL_SEP  0xCCCCCC
#define COL_TEXT_TITLE 0x222222
#define COL_TEXT_SUB   0x666666
#define COL_TEXT_LABEL 0x555555
#define COL_INPUT_BDR  0xCCCCCC
#define COL_SHOW_BG    0xEEEEEE
#define COL_SHOW_BDR   0xCCCCCC
#define COL_BTN_CONN   0x2196F3
#define COL_BTN_CANCEL 0x757575

// ── Container helper ──────────────────────────────────────────────────────────

static lv_obj_t *make_cont(lv_obj_t *parent)
{
	lv_obj_t *c = lv_obj_create(parent);
	lv_obj_set_style_bg_opa(c, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_border_width(c, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(c, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(c, 0, LV_PART_MAIN);
	lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
	return c;
}

// ── Event callbacks ───────────────────────────────────────────────────────────

static void on_back_clicked(lv_event_t *e)
{
	(void)e;
	if (s_keyboard) lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
	if (s_nav_back) lv_scr_load(s_nav_back);
}

static void on_connect_clicked(lv_event_t *e)
{
	(void)e;
	ESP_LOGI(TAG, "Send SSID+password to Base via BLE → Connecting [F]");
	if (s_keyboard) lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void on_cancel_clicked(lv_event_t *e)
{
	(void)e;
	if (s_keyboard) lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
	if (s_nav_back) lv_scr_load(s_nav_back);
}

static void on_show_clicked(lv_event_t *e)
{
	(void)e;
	if (!s_pwd_ta || !s_show_lbl) return;
	bool pm = lv_textarea_get_password_mode(s_pwd_ta);
	lv_textarea_set_password_mode(s_pwd_ta, !pm);
	lv_label_set_text(s_show_lbl, pm ? "Hide" : "Show");
}

static void on_ta_focused(lv_event_t *e)
{
	(void)e;
	if (s_keyboard) lv_obj_clear_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void on_ta_defocused(lv_event_t *e)
{
	(void)e;
	if (s_keyboard) lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
}

static void on_kbd_event(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	if (code == LV_EVENT_READY) {
		ESP_LOGI(TAG, "Keyboard ready → Connect [F]");
		if (s_keyboard) lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
	} else if (code == LV_EVENT_CANCEL) {
		if (s_keyboard) lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
	}
}

// ── Status bar ────────────────────────────────────────────────────────────────

static void build_status_bar(lv_obj_t *parent)
{
	lv_obj_t *bar = lv_obj_create(parent);
	lv_obj_set_size(bar, LV_PCT(100), STATUS_BAR_H);
	lv_obj_set_style_bg_color(bar, lv_color_hex(COL_STATUS_BG), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(bar, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(bar, 0, LV_PART_MAIN);
	lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *batt = lv_label_create(bar);
	lv_label_set_text(batt, "--%");   // [F]
	lv_obj_set_style_text_font(batt, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(batt, lv_color_hex(0x546E7A), LV_PART_MAIN);
	lv_obj_align(batt, LV_ALIGN_RIGHT_MID, 0, 0);
}

// ── Header bar ────────────────────────────────────────────────────────────────

static void build_header_bar(lv_obj_t *parent)
{
	lv_obj_t *hdr = lv_obj_create(parent);
	lv_obj_set_size(hdr, LV_PCT(100), HEADER_BAR_H);
	lv_obj_set_style_bg_color(hdr, lv_color_hex(COL_HEADER_BG), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(hdr, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_width(hdr, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(hdr, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(hdr, 16, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(hdr, 0, LV_PART_MAIN);
	lv_obj_clear_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_layout(hdr, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(hdr, 16, LV_PART_MAIN);

	lv_obj_t *back = lv_label_create(hdr);
	lv_label_set_text(back, LV_SYMBOL_LEFT);
	lv_obj_set_style_text_font(back, &lv_font_montserrat_24, LV_PART_MAIN);
	lv_obj_set_style_text_color(back, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_obj_add_flag(back, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_pad_all(back, 4, LV_PART_MAIN);
	lv_obj_add_event_cb(back, on_back_clicked, LV_EVENT_CLICKED, NULL);

	lv_obj_t *title = lv_label_create(hdr);
	lv_label_set_text(title, "Enter Password");
	lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

// ── Left panel — connecting indicator ────────────────────────────────────────
// Shows while Base attempts to connect (static placeholder until BLE live).

static void build_left_panel(lv_obj_t *parent)
{
	lv_obj_t *panel = lv_obj_create(parent);
	lv_obj_set_flex_grow(panel, 1);
	lv_obj_set_height(panel, LV_PCT(100));
	lv_obj_set_style_bg_color(panel, lv_color_hex(COL_LEFT_BG), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_side(panel, LV_BORDER_SIDE_RIGHT, LV_PART_MAIN);
	lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(panel, lv_color_hex(COL_PANEL_SEP), LV_PART_MAIN);
	lv_obj_set_style_radius(panel, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(panel, 20, LV_PART_MAIN);
	lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

	// Centred column
	lv_obj_t *col = make_cont(panel);
	lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_align(col, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_layout(col, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_row(col, 8, LV_PART_MAIN);

	lv_obj_t *h = lv_label_create(col);
	lv_label_set_text(h, "Connecting...");
	lv_obj_set_style_text_font(h, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(h, lv_color_hex(COL_TEXT_LABEL), LV_PART_MAIN);

	lv_obj_t *s = lv_label_create(col);
	lv_label_set_text(s, "Waiting for Base to confirm");
	lv_obj_set_style_text_font(s, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(s, lv_color_hex(0x888888), LV_PART_MAIN);
}

// ── Right panel — password entry form ────────────────────────────────────────

static void build_right_panel(lv_obj_t *parent)
{
	lv_obj_t *panel = lv_obj_create(parent);
	lv_obj_set_flex_grow(panel, 1);
	lv_obj_set_height(panel, LV_PCT(100));
	lv_obj_set_style_bg_color(panel, lv_color_hex(COL_WHITE), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(panel, 0, LV_PART_MAIN);
	// Pad top ~60px to roughly centre the form visually when keyboard is hidden,
	// while keeping elements above the keyboard when it is shown.
	lv_obj_set_style_pad_top(panel, 60, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(panel, 28, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(panel, 20, LV_PART_MAIN);
	lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(panel, 0, LV_PART_MAIN);

	// "Enter Password" title
	lv_obj_t *title = lv_label_create(panel);
	lv_label_set_text(title, "Enter Password");
	lv_obj_set_style_text_font(title, &lv_font_montserrat_24, LV_PART_MAIN);
	lv_obj_set_style_text_color(title, lv_color_hex(COL_TEXT_TITLE), LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(title, 12, LV_PART_MAIN);

	// "Network: --- · Channel: ---"  [F]
	lv_obj_t *sub = lv_label_create(panel);
	lv_label_set_text(sub, "Network: --- \xC2\xB7 Channel: ---");  // [F] filled on entry
	lv_obj_set_style_text_font(sub, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(sub, lv_color_hex(COL_TEXT_SUB), LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(sub, 20, LV_PART_MAIN);

	// "WiFi Password" field label
	lv_obj_t *field_lbl = lv_label_create(panel);
	lv_label_set_text(field_lbl, "WiFi Password");
	lv_obj_set_style_text_font(field_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(field_lbl, lv_color_hex(COL_TEXT_LABEL), LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(field_lbl, 8, LV_PART_MAIN);

	// Input row: textarea + Show button
	lv_obj_t *input_row = make_cont(panel);
	lv_obj_set_size(input_row, LV_PCT(100), 48);
	lv_obj_set_layout(input_row, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(input_row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(input_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(input_row, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(input_row, 16, LV_PART_MAIN);

	// Password textarea
	s_pwd_ta = lv_textarea_create(input_row);
	lv_obj_set_flex_grow(s_pwd_ta, 1);
	lv_obj_set_height(s_pwd_ta, 48);
	lv_textarea_set_one_line(s_pwd_ta, true);
	lv_textarea_set_password_mode(s_pwd_ta, true);
	lv_textarea_set_placeholder_text(s_pwd_ta, "Enter password...");
	lv_textarea_set_max_length(s_pwd_ta, 64);
	lv_obj_set_style_text_font(s_pwd_ta, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_border_width(s_pwd_ta, 2, LV_PART_MAIN);
	lv_obj_set_style_border_color(s_pwd_ta, lv_color_hex(COL_INPUT_BDR), LV_PART_MAIN);
	lv_obj_set_style_border_color(s_pwd_ta, lv_color_hex(0x2196F3),
	                               LV_PART_MAIN | LV_STATE_FOCUSED);
	lv_obj_set_style_radius(s_pwd_ta, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(s_pwd_ta, 12, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(s_pwd_ta, 0, LV_PART_MAIN);
	lv_obj_add_event_cb(s_pwd_ta, on_ta_focused,   LV_EVENT_FOCUSED,   NULL);
	lv_obj_add_event_cb(s_pwd_ta, on_ta_defocused, LV_EVENT_DEFOCUSED, NULL);

	// "Show" toggle button
	lv_obj_t *show_btn = lv_button_create(input_row);
	lv_obj_set_size(show_btn, 80, 48);
	lv_obj_set_style_bg_color(show_btn, lv_color_hex(COL_SHOW_BG), LV_PART_MAIN);
	lv_obj_set_style_border_width(show_btn, 2, LV_PART_MAIN);
	lv_obj_set_style_border_color(show_btn, lv_color_hex(COL_SHOW_BDR), LV_PART_MAIN);
	lv_obj_set_style_radius(show_btn, 8, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(show_btn, 0, LV_PART_MAIN);
	lv_obj_add_event_cb(show_btn, on_show_clicked, LV_EVENT_CLICKED, NULL);
	s_show_lbl = lv_label_create(show_btn);
	lv_label_set_text(s_show_lbl, "Show");
	lv_obj_set_style_text_font(s_show_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
	lv_obj_set_style_text_color(s_show_lbl, lv_color_hex(COL_TEXT_TITLE), LV_PART_MAIN);
	lv_obj_center(s_show_lbl);

	// Action buttons row: Connect + Cancel
	lv_obj_t *btn_row = make_cont(panel);
	lv_obj_set_size(btn_row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_layout(btn_row, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(btn_row, 12, LV_PART_MAIN);
	lv_obj_set_style_pad_top(btn_row, 8, LV_PART_MAIN);

	lv_obj_t *conn_btn = lv_button_create(btn_row);
	lv_obj_set_size(conn_btn, LV_SIZE_CONTENT, 52);
	lv_obj_set_style_bg_color(conn_btn, lv_color_hex(COL_BTN_CONN), LV_PART_MAIN);
	lv_obj_set_style_radius(conn_btn, 8, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(conn_btn, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(conn_btn, 32, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(conn_btn, 0, LV_PART_MAIN);
	lv_obj_add_event_cb(conn_btn, on_connect_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_t *conn_lbl = lv_label_create(conn_btn);
	lv_label_set_text(conn_lbl, "Connect");
	lv_obj_set_style_text_font(conn_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(conn_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_obj_center(conn_lbl);

	lv_obj_t *cancel_btn = lv_button_create(btn_row);
	lv_obj_set_size(cancel_btn, LV_SIZE_CONTENT, 52);
	lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(COL_BTN_CANCEL), LV_PART_MAIN);
	lv_obj_set_style_radius(cancel_btn, 8, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(cancel_btn, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(cancel_btn, 32, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(cancel_btn, 0, LV_PART_MAIN);
	lv_obj_add_event_cb(cancel_btn, on_cancel_clicked, LV_EVENT_CLICKED, NULL);
	lv_obj_t *cancel_lbl = lv_label_create(cancel_btn);
	lv_label_set_text(cancel_lbl, "Cancel");
	lv_obj_set_style_text_font(cancel_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(cancel_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_obj_center(cancel_lbl);
}

// ── Public API ────────────────────────────────────────────────────────────────

lv_obj_t *screen_password_entry_create(const app_theme_t *t)
{
	(void)t;

	// Reset statics — this screen may be re-entered
	s_keyboard = NULL;
	s_pwd_ta   = NULL;
	s_show_lbl = NULL;

	lv_obj_t *scr = lv_obj_create(NULL);
	lv_obj_set_style_bg_color(scr, lv_color_hex(COL_WHITE), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_pad_all(scr, 0, LV_PART_MAIN);
	lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(scr, 0, LV_PART_MAIN);
	lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_layout(scr, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(scr, 0, LV_PART_MAIN);

	build_status_bar(scr);
	build_header_bar(scr);

	// Main area: flex ROW (left + right panels)
	lv_obj_t *main_area = make_cont(scr);
	lv_obj_set_width(main_area, LV_PCT(100));
	lv_obj_set_flex_grow(main_area, 1);
	lv_obj_set_layout(main_area, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(main_area, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(main_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

	build_left_panel(main_area);
	build_right_panel(main_area);

	// Keyboard — floating over the screen, hidden until textarea gets focus.
	// LV_OBJ_FLAG_FLOATING excludes it from the flex layout of scr.
	s_keyboard = lv_keyboard_create(scr);
	lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_FLOATING);
	lv_obj_align(s_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_keyboard_set_textarea(s_keyboard, s_pwd_ta);
	lv_keyboard_set_mode(s_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_obj_add_flag(s_keyboard, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_event_cb(s_keyboard, on_kbd_event, LV_EVENT_READY,  NULL);
	lv_obj_add_event_cb(s_keyboard, on_kbd_event, LV_EVENT_CANCEL, NULL);

	ESP_LOGI(TAG, "Password Entry screen created");
	return scr;
}
