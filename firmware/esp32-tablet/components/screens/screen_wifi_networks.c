#include "screen_wifi_networks.h"
#include "esp_log.h"

static const char *TAG = "screen_wifi";

static lv_obj_t *s_nav_back     = NULL;
static lv_obj_t *s_nav_password = NULL;

void screen_wifi_networks_set_nav_back(lv_obj_t *base_config_scr)
{
	s_nav_back = base_config_scr;
}

void screen_wifi_networks_set_nav_password(lv_obj_t *pwd_scr)
{
	s_nav_password = pwd_scr;
}

// ── Layout constants ──────────────────────────────────────────────────────────

#define STATUS_BAR_H  28
#define HEADER_BAR_H  52
#define LEFT_PANEL_W  340

// ── Colours ───────────────────────────────────────────────────────────────────

#define COL_STATUS_BG     0x111111
#define COL_HEADER_BG     0x222222
#define COL_WHITE         0xFFFFFF
#define COL_LEFT_BG       0xF5F5F5
#define COL_PANEL_SEP     0xCCCCCC
#define COL_ROW_BORDER    0xF0F0F0
#define COL_TEXT_TITLE    0x222222
#define COL_TEXT_LABEL    0x555555
#define COL_TEXT_VALUE    0x333333
#define COL_TEXT_META     0xAAAAAA
#define COL_TEXT_DIM      0xCCCCCC
#define COL_SECTION_HDR   0xAAAAAA
#define COL_DOT_GREEN     0x43A047
#define COL_DOT_GREY      0xE0E0E0
#define COL_BTN_CONNECT   0x2196F3
#define COL_BTN_DISC      0xE53935
#define COL_FORGET_BORDER 0xFFCDD2
#define COL_FORGET_TEXT   0xE53935
#define COL_SCAN_TEXT     0x2196F3
#define COL_SCAN_BORDER   0xBBDEFB

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
	if (s_nav_back) lv_scr_load(s_nav_back);
}

static void on_disconnect_clicked(lv_event_t *e)
{
	(void)e;
	ESP_LOGI(TAG, "Disconnect [F]");
}

static void on_connect_rem_clicked(lv_event_t *e)
{
	(void)e;
	ESP_LOGI(TAG, "Connect remembered → Connecting [F]");
}

static void on_forget_clicked(lv_event_t *e)
{
	(void)e;
	ESP_LOGI(TAG, "Forget network [F]");
}

static void on_connect_other_clicked(lv_event_t *e)
{
	(void)e;
	ESP_LOGI(TAG, "Connect other → Password Entry");
	if (s_nav_password) lv_scr_load(s_nav_password);
}

static void on_scan_clicked(lv_event_t *e)
{
	(void)e;
	ESP_LOGI(TAG, "WiFi scan [F]");
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
	lv_label_set_text(title, "WiFi Networks");
	lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

// ── Left panel ────────────────────────────────────────────────────────────────

static void add_table_row(lv_obj_t *parent,
                          const char *label_text,
                          const char *value_text,
                          lv_color_t  value_col)
{
	lv_obj_t *row = make_cont(parent);
	lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_layout(row, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_ver(row, 5, LV_PART_MAIN);

	lv_obj_t *lbl = lv_label_create(row);
	lv_label_set_text(lbl, label_text);
	lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
	lv_obj_set_style_text_color(lbl, lv_color_hex(COL_TEXT_LABEL), LV_PART_MAIN);
	lv_obj_set_width(lbl, 140);

	lv_obj_t *val = lv_label_create(row);
	lv_label_set_text(val, value_text);
	lv_obj_set_style_text_font(val, &lv_font_montserrat_14, LV_PART_MAIN);
	lv_obj_set_style_text_color(val, value_col, LV_PART_MAIN);
	lv_obj_set_flex_grow(val, 1);
}

static void build_left_panel(lv_obj_t *parent)
{
	lv_obj_t *panel = lv_obj_create(parent);
	lv_obj_set_size(panel, LEFT_PANEL_W, LV_PCT(100));
	lv_obj_set_style_bg_color(panel, lv_color_hex(COL_LEFT_BG), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_side(panel, LV_BORDER_SIDE_RIGHT, LV_PART_MAIN);
	lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(panel, lv_color_hex(COL_PANEL_SEP), LV_PART_MAIN);
	lv_obj_set_style_radius(panel, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_all(panel, 20, LV_PART_MAIN);
	lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(panel, 0, LV_PART_MAIN);

	// "Current Config" title
	lv_obj_t *t1 = lv_label_create(panel);
	lv_label_set_text(t1, "Current Config");
	lv_obj_set_style_text_font(t1, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(t1, lv_color_hex(COL_TEXT_TITLE), LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(t1, 10, LV_PART_MAIN);

	// [F] all values placeholder until BLE fetch live
	add_table_row(panel, "Network SSID:", "---", lv_color_hex(COL_TEXT_VALUE));
	add_table_row(panel, "Channel:",      "---", lv_color_hex(COL_TEXT_VALUE));
	add_table_row(panel, "Status:",       "---", lv_color_hex(COL_TEXT_VALUE));
	add_table_row(panel, "IP Address:",   "---", lv_color_hex(COL_TEXT_VALUE));
	add_table_row(panel, "Signal:",       "---", lv_color_hex(COL_TEXT_VALUE));

	// "Remembered" title
	lv_obj_t *t2 = lv_label_create(panel);
	lv_label_set_text(t2, "Remembered");
	lv_obj_set_style_text_font(t2, &lv_font_montserrat_20, LV_PART_MAIN);
	lv_obj_set_style_text_color(t2, lv_color_hex(COL_TEXT_TITLE), LV_PART_MAIN);
	lv_obj_set_style_pad_top(t2, 20, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(t2, 10, LV_PART_MAIN);

	add_table_row(panel, "Slots used:", "--- of 5", lv_color_hex(COL_TEXT_VALUE));

	lv_obj_t *info = lv_label_create(panel);
	lv_label_set_text(info,
		"On power-on, Base connects automatically to the "
		"highest-signal remembered network in range.");
	lv_obj_set_style_text_font(info, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(info, lv_color_hex(COL_TEXT_META), LV_PART_MAIN);
	lv_label_set_long_mode(info, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(info, LV_PCT(100));
	lv_obj_set_style_pad_top(info, 8, LV_PART_MAIN);
}

// ── Right panel — shared helpers ──────────────────────────────────────────────

// Base container for a network row.
static lv_obj_t *build_net_row(lv_obj_t *parent, bool last)
{
	lv_obj_t *row = lv_obj_create(parent);
	lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_radius(row, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(row, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(row, 9, LV_PART_MAIN);
	lv_obj_set_style_min_height(row, 44, LV_PART_MAIN);
	lv_obj_set_style_border_side(row, last ? LV_BORDER_SIDE_NONE : LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
	lv_obj_set_style_border_width(row, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(row, lv_color_hex(COL_ROW_BORDER), LV_PART_MAIN);
	lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_layout(row, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(row, 10, LV_PART_MAIN);
	return row;
}

static void add_dot(lv_obj_t *parent, lv_color_t col)
{
	lv_obj_t *dot = lv_obj_create(parent);
	lv_obj_set_size(dot, 10, 10);
	lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
	lv_obj_set_style_bg_color(dot, col, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
	lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
}

static void add_ssid_col(lv_obj_t *parent,
                         const char *ssid,
                         const char *meta,
                         lv_color_t  ssid_col,
                         lv_color_t  meta_col)
{
	lv_obj_t *col = make_cont(parent);
	lv_obj_set_flex_grow(col, 1);
	lv_obj_set_height(col, LV_SIZE_CONTENT);
	lv_obj_set_layout(col, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(col, 2, LV_PART_MAIN);

	lv_obj_t *s = lv_label_create(col);
	lv_label_set_text(s, ssid);
	lv_obj_set_style_text_font(s, &lv_font_montserrat_14, LV_PART_MAIN);
	lv_obj_set_style_text_color(s, ssid_col, LV_PART_MAIN);

	lv_obj_t *m = lv_label_create(col);
	lv_label_set_text(m, meta);
	lv_obj_set_style_text_font(m, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(m, meta_col, LV_PART_MAIN);
}

// Filled action button: blue Connect or red Disconnect.
static void add_action_btn(lv_obj_t *parent,
                           const char *text,
                           lv_color_t  bg_col,
                           lv_event_cb_t cb)
{
	lv_obj_t *btn = lv_button_create(parent);
	lv_obj_set_size(btn, LV_SIZE_CONTENT, 34);
	lv_obj_set_style_bg_color(btn, bg_col, LV_PART_MAIN);
	lv_obj_set_style_radius(btn, 6, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(btn, 14, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(btn, 0, LV_PART_MAIN);
	lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t *lbl = lv_label_create(btn);
	lv_label_set_text(lbl, text);
	lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
	lv_obj_center(lbl);
}

// Outlined "Forget" button: transparent bg, red border + text.
static void add_forget_btn(lv_obj_t *parent, lv_event_cb_t cb)
{
	lv_obj_t *btn = lv_button_create(parent);
	lv_obj_set_size(btn, LV_SIZE_CONTENT, 28);
	lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_STATE_PRESSED | LV_PART_MAIN);
	lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(btn, lv_color_hex(COL_FORGET_BORDER), LV_PART_MAIN);
	lv_obj_set_style_radius(btn, 5, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(btn, 8, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(btn, 0, LV_PART_MAIN);
	lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

	lv_obj_t *lbl = lv_label_create(btn);
	lv_label_set_text(lbl, "Forget");
	lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(lbl, lv_color_hex(COL_FORGET_TEXT), LV_PART_MAIN);
	lv_obj_center(lbl);
}

// Section header row with optional Scan button on the right.
static void build_section_hdr(lv_obj_t *parent, const char *text, bool add_scan)
{
	lv_obj_t *row = make_cont(parent);
	lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_layout(row, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_top(row, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(row, 5, LV_PART_MAIN);

	lv_obj_t *lbl = lv_label_create(row);
	lv_label_set_text(lbl, text);
	lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(lbl, lv_color_hex(COL_SECTION_HDR), LV_PART_MAIN);
	lv_obj_set_flex_grow(lbl, 1);

	if (!add_scan) return;

	lv_obj_t *scan = lv_button_create(row);
	lv_obj_set_size(scan, LV_SIZE_CONTENT, 26);
	lv_obj_set_style_bg_opa(scan, LV_OPA_TRANSP, LV_PART_MAIN);
	lv_obj_set_style_bg_opa(scan, LV_OPA_TRANSP, LV_STATE_PRESSED | LV_PART_MAIN);
	lv_obj_set_style_border_width(scan, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(scan, lv_color_hex(COL_SCAN_BORDER), LV_PART_MAIN);
	lv_obj_set_style_radius(scan, 5, LV_PART_MAIN);
	lv_obj_set_style_shadow_width(scan, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(scan, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(scan, 0, LV_PART_MAIN);
	lv_obj_add_event_cb(scan, on_scan_clicked, LV_EVENT_CLICKED, NULL);

	lv_obj_t *scan_lbl = lv_label_create(scan);
	lv_label_set_text(scan_lbl, "Scan " LV_SYMBOL_REFRESH);
	lv_obj_set_style_text_font(scan_lbl, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(scan_lbl, lv_color_hex(COL_SCAN_TEXT), LV_PART_MAIN);
	lv_obj_center(scan_lbl);
}

// ── Right panel ───────────────────────────────────────────────────────────────

static void build_right_panel(lv_obj_t *parent)
{
	lv_obj_t *panel = lv_obj_create(parent);
	lv_obj_set_flex_grow(panel, 1);
	lv_obj_set_height(panel, LV_PCT(100));
	lv_obj_set_style_bg_color(panel, lv_color_hex(COL_WHITE), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN);
	lv_obj_set_style_radius(panel, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_hor(panel, 20, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(panel, 16, LV_PART_MAIN);
	lv_obj_set_scroll_dir(panel, LV_DIR_VER);
	lv_obj_set_layout(panel, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_row(panel, 0, LV_PART_MAIN);

	// ── Remembered Networks ────────────────────────────────────────────────
	build_section_hdr(panel, "REMEMBERED NETWORKS", false);

	// Row: currently connected  [F]
	lv_obj_t *r1 = build_net_row(panel, false);
	add_dot(r1, lv_color_hex(COL_DOT_GREEN));
	add_ssid_col(r1,
		"---",             // [F] SSID from BLE
		"Ch -- \xC2\xB7 -- dBm",
		lv_color_hex(COL_TEXT_META),   // greyed, mimics italic connected state
		lv_color_hex(COL_TEXT_META));
	add_action_btn(r1, "Disconnect", lv_color_hex(COL_BTN_DISC), on_disconnect_clicked);

	// Row: remembered, in range  [F]
	lv_obj_t *r2 = build_net_row(panel, false);
	add_dot(r2, lv_color_hex(COL_DOT_GREY));
	add_ssid_col(r2,
		"---",
		"Ch -- \xC2\xB7 -- dBm",
		lv_color_hex(COL_TEXT_VALUE),
		lv_color_hex(COL_TEXT_META));
	add_action_btn(r2, "Connect", lv_color_hex(COL_BTN_CONNECT), on_connect_rem_clicked);
	add_forget_btn(r2, on_forget_clicked);

	// Row: remembered, not in range  [F]
	lv_obj_t *r3 = build_net_row(panel, true);
	add_dot(r3, lv_color_hex(COL_DOT_GREY));
	add_ssid_col(r3,
		"---",
		"Ch -- \xC2\xB7 --",
		lv_color_hex(COL_TEXT_DIM),
		lv_color_hex(COL_TEXT_DIM));
	lv_obj_t *nr = lv_label_create(r3);
	lv_label_set_text(nr, "Not in range");
	lv_obj_set_style_text_font(nr, &lv_font_montserrat_12, LV_PART_MAIN);
	lv_obj_set_style_text_color(nr, lv_color_hex(COL_TEXT_DIM), LV_PART_MAIN);
	add_forget_btn(r3, on_forget_clicked);

	// ── Other Networks ─────────────────────────────────────────────────────
	build_section_hdr(panel, "OTHER NETWORKS", true);

	// [F] placeholder scan results
	lv_obj_t *o1 = build_net_row(panel, false);
	add_ssid_col(o1,
		"---",
		"Ch -- \xC2\xB7 -- dBm",
		lv_color_hex(COL_TEXT_VALUE),
		lv_color_hex(COL_TEXT_META));
	add_action_btn(o1, "Connect", lv_color_hex(COL_BTN_CONNECT), on_connect_other_clicked);

	lv_obj_t *o2 = build_net_row(panel, true);
	add_ssid_col(o2,
		"---",
		"Ch -- \xC2\xB7 -- dBm",
		lv_color_hex(COL_TEXT_VALUE),
		lv_color_hex(COL_TEXT_META));
	add_action_btn(o2, "Connect", lv_color_hex(COL_BTN_CONNECT), on_connect_other_clicked);
}

// ── Public API ────────────────────────────────────────────────────────────────

lv_obj_t *screen_wifi_networks_create(const app_theme_t *t)
{
	(void)t;

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

	// Main area: flex ROW containing left and right panels
	lv_obj_t *main_area = make_cont(scr);
	lv_obj_set_width(main_area, LV_PCT(100));
	lv_obj_set_flex_grow(main_area, 1);
	lv_obj_set_layout(main_area, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(main_area, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(main_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

	build_left_panel(main_area);
	build_right_panel(main_area);

	ESP_LOGI(TAG, "WiFi Networks screen created");
	return scr;
}
