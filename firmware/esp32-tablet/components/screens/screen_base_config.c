#include "screen_base_config.h"
#include "esp_log.h"

static const char *TAG = "screen_base_cfg";

// Back-navigation target (set after both screens are created in main)
static lv_obj_t *s_nav_back = NULL;
static lv_obj_t *s_nav_wifi = NULL;

void screen_base_config_set_nav_back(lv_obj_t *home_scr)
{
    s_nav_back = home_scr;
}

void screen_base_config_set_nav_wifi(lv_obj_t *wifi_scr)
{
    s_nav_wifi = wifi_scr;
}

// ── Layout constants ──────────────────────────────────────────────────────────

#define STATUS_BAR_H   28
#define HEADER_BAR_H   52
#define POS_BANNER_H   44
#define CONFIG_STRIP_H 72
// Cards area fills remaining: 600 - 28 - 52 - 44 - 72 = 404px

// ── Colours (light-mode specific — not drawn from theme palette) ──────────────

#define COL_STATUS_BG     0x111111
#define COL_HEADER_BG     0x222222
#define COL_WHITE         0xFFFFFF
#define COL_LIGHT_BG      0xFAFAFA
#define COL_BORDER        0xE0E0E0
#define COL_STATUS_BOX_BG 0xF5F5F5
#define COL_TEXT_BODY     0x333333
#define COL_TEXT_MUTED    0x888888
#define COL_TEXT_SUB      0x666666
#define COL_HEADER_TEXT   0xFFFFFF
#define COL_BANNER_LABEL  0x888888

// Mode card accent colours
#define COL_SURVEYIN_TITLE  0x1565C0
#define COL_SURVEYIN_BTN    0x2196F3
#define COL_RTCM_TITLE      0x2E7D32
#define COL_RTCM_BTN        0x43A047
#define COL_SAT_TITLE       0x6A1B9A
#define COL_SAT_BTN         0x8E24AA

// Position badge colours
#define COL_BADGE_NONE_BG   0xFFEBEE
#define COL_BADGE_NONE_TEXT 0xC62828

// ── Shared container helper ───────────────────────────────────────────────────

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

static void on_pos_banner_clicked(lv_event_t *e)
{
    (void)e;
    ESP_LOGI(TAG, "Navigate → Fixed Position entry");
    // TODO: lv_scr_load(screen_base_fixed_pos_create())
}

static void on_surveyin_clicked(lv_event_t *e)
{
    (void)e;
    ESP_LOGI(TAG, "Navigate → Survey-in");
    // TODO: lv_scr_load(screen_base_surveyin_create())
}

static void on_rtcm_clicked(lv_event_t *e)
{
    (void)e;
    ESP_LOGI(TAG, "Navigate → RTCM Data streaming");
    // TODO: lv_scr_load(screen_base_streaming_create())
}

static void on_sat_clicked(lv_event_t *e)
{
    (void)e;
    ESP_LOGI(TAG, "Navigate → Satellite View");
    // TODO: lv_scr_load(screen_base_satview_create())
}

static void on_wifi_clicked(lv_event_t *e)
{
    (void)e;
    ESP_LOGI(TAG, "Navigate → WiFi Networks");
    if (s_nav_wifi) lv_scr_load(s_nav_wifi);
}

static void on_fixedpos_clicked(lv_event_t *e)
{
    (void)e;
    ESP_LOGI(TAG, "Navigate → Fixed Position entry");
    // TODO: lv_scr_load(screen_base_fixed_pos_create())
}

// ── Status bar ────────────────────────────────────────────────────────────────
// Dark strip at top — only tablet battery on this screen (Base context).
// [F] battery value until ADC read is live.

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
    lv_label_set_text(batt, "--%");       // [F] ADC read not wired yet
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

    // Back button
    lv_obj_t *back = lv_label_create(hdr);
    lv_label_set_text(back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_font(back, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(back, lv_color_hex(COL_HEADER_TEXT), LV_PART_MAIN);
    lv_obj_add_flag(back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_pad_all(back, 4, LV_PART_MAIN); // larger tap target
    lv_obj_add_event_cb(back, on_back_clicked, LV_EVENT_CLICKED, NULL);

    // Title
    lv_obj_t *title = lv_label_create(hdr);
    lv_label_set_text(title, "Base Configuration");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(COL_HEADER_TEXT), LV_PART_MAIN);
}

// ── Position source banner ────────────────────────────────────────────────────
// [F] Shows "No position set" until BLE live data arrives.
// Tappable → Fixed Position entry screen.

static void build_pos_banner(lv_obj_t *parent)
{
    lv_obj_t *banner = lv_obj_create(parent);
    lv_obj_set_size(banner, LV_PCT(100), POS_BANNER_H);
    lv_obj_set_style_bg_color(banner, lv_color_hex(COL_WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(banner, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_side(banner, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_width(banner, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(banner, lv_color_hex(COL_BORDER), LV_PART_MAIN);
    lv_obj_set_style_radius(banner, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(banner, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(banner, 0, LV_PART_MAIN);
    lv_obj_clear_flag(banner, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(banner, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(banner, on_pos_banner_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_set_layout(banner, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(banner, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(banner, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(banner, 12, LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(banner);
    lv_label_set_text(lbl, "Base Position:");
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl, lv_color_hex(COL_BANNER_LABEL), LV_PART_MAIN);

    // [F] badge — "No position set" (red) until BLE position source live
    lv_obj_t *badge = lv_obj_create(banner);
    lv_obj_set_size(badge, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(badge, 20, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge, lv_color_hex(COL_BADGE_NONE_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(badge, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(badge, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(badge, 4, LV_PART_MAIN);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *badge_lbl = lv_label_create(badge);
    lv_label_set_text(badge_lbl, "No position set"); // [F]
    lv_obj_set_style_text_font(badge_lbl, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(badge_lbl, lv_color_hex(COL_BADGE_NONE_TEXT), LV_PART_MAIN);
    lv_obj_center(badge_lbl);

    lv_obj_t *hint = lv_label_create(banner);
    lv_label_set_text(hint, "\xC2\xB7  tap to change " LV_SYMBOL_RIGHT); // · tap to change ›
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(hint, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
}

// ── Mode card ─────────────────────────────────────────────────────────────────
// Spec note: do NOT use flex to bottom-anchor the button — use lv_obj_align so
// all three buttons sit at the same Y regardless of description text length.

static void build_mode_card(lv_obj_t *parent,
                             const char *title,
                             lv_color_t  title_col,
                             const char *desc,
                             const char *status_text,  // [F] placeholder
                             const char *btn_text,
                             lv_color_t  btn_col,
                             lv_event_cb_t btn_cb)
{
    // Card — no LVGL layout; children are positioned absolutely
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_flex_grow(card, 1);
    lv_obj_set_height(card, LV_PCT(100));
    lv_obj_set_style_bg_color(card, lv_color_hex(COL_WHITE), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(COL_BORDER), LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 24, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    // No lv_obj_set_layout — children positioned with lv_obj_set_pos / lv_obj_align

    // Content container (flex column, auto height from top of card content area)
    lv_obj_t *content = make_cont(card);
    lv_obj_set_pos(content, 0, 0);        // top-left of content area (inside pad)
    lv_obj_set_size(content, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_layout(content, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(content, 8, LV_PART_MAIN);

    // Title (e.g. "⊕ Survey-in")
    lv_obj_t *title_lbl = lv_label_create(content);
    lv_label_set_text(title_lbl, title);
    lv_obj_set_style_text_font(title_lbl, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(title_lbl, title_col, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(title_lbl, 0, LV_PART_MAIN);

    // Description
    lv_obj_t *desc_lbl = lv_label_create(content);
    lv_label_set_text(desc_lbl, desc);
    lv_obj_set_style_text_font(desc_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(desc_lbl, lv_color_hex(COL_TEXT_SUB), LV_PART_MAIN);
    lv_label_set_long_mode(desc_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc_lbl, LV_PCT(100));

    // Status box
    lv_obj_t *status_box = lv_obj_create(content);
    lv_obj_set_width(status_box, LV_PCT(100));
    lv_obj_set_height(status_box, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(status_box, lv_color_hex(COL_STATUS_BOX_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(status_box, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(status_box, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(status_box, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(status_box, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(status_box, 10, LV_PART_MAIN);
    lv_obj_clear_flag(status_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *status_lbl = lv_label_create(status_box);
    lv_label_set_text(status_lbl, status_text);
    lv_obj_set_style_text_font(status_lbl, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(status_lbl, lv_color_hex(COL_TEXT_MUTED), LV_PART_MAIN);
    lv_label_set_long_mode(status_lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(status_lbl, LV_PCT(100));

    // Button — anchored to bottom-centre of card content area (spec note)
    lv_obj_t *btn = lv_button_create(card);
    lv_obj_set_size(btn, LV_PCT(100), 56);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn, btn_col, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, btn_text);
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(btn_lbl);
}

// ── Vertical divider ──────────────────────────────────────────────────────────

static void build_divider(lv_obj_t *parent)
{
    lv_obj_t *div = lv_obj_create(parent);
    lv_obj_set_size(div, 2, LV_PCT(100));
    lv_obj_set_style_bg_color(div, lv_color_hex(COL_BORDER), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(div, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(div, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(div, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(div, 0, LV_PART_MAIN);
    lv_obj_clear_flag(div, LV_OBJ_FLAG_SCROLLABLE);
}

// ── Cards area ────────────────────────────────────────────────────────────────

static void build_cards_area(lv_obj_t *parent)
{
    lv_obj_t *area = make_cont(parent);
    lv_obj_set_width(area, LV_PCT(100));
    lv_obj_set_flex_grow(area, 1);
    lv_obj_set_layout(area, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(area, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_right(area, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_top(area, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(area, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_column(area, 0, LV_PART_MAIN);

    // Survey-in card — [F] status until F9P + BLE live
    build_mode_card(area,
        "Survey-in",
        lv_color_hex(COL_SURVEYIN_TITLE),
        "The Base unit surveys its own position by averaging GNSS measurements "
        "over time. When the target accuracy is reached it locks its position "
        "and begins streaming RTCM corrections.",
        "Never completed",                   // [F] BLE fetch on screen open
        "Start Survey-in  " LV_SYMBOL_RIGHT,
        lv_color_hex(COL_SURVEYIN_BTN),
        on_surveyin_clicked);

    build_divider(area);

    // RTCM Data card — [F] status until SX1262 + BLE live
    build_mode_card(area,
        "RTCM Data",
        lv_color_hex(COL_RTCM_TITLE),
        "The Base unit broadcasts RTCM correction data to the Rover over the "
        "LoRa radio link. Requires a completed survey-in or a known fixed position.",
        "Not streaming",                     // [F]
        "View RTCM Data  " LV_SYMBOL_RIGHT,
        lv_color_hex(COL_RTCM_BTN),
        on_rtcm_clicked);

    build_divider(area);

    // Satellite View card — [F] status until F9P + BLE live
    build_mode_card(area,
        "Satellite View",
        lv_color_hex(COL_SAT_TITLE),
        "Live sky plot from the Base GNSS receiver — satellite positions, "
        "signal strength and fix quality. Use this to verify sky visibility "
        "and signal health before starting a survey-in.",
        "0 sats tracked  \xC2\xB7  No fix",  // [F] · U+00B7
        "View Satellites  " LV_SYMBOL_RIGHT,
        lv_color_hex(COL_SAT_BTN),
        on_sat_clicked);
}

// ── Config strip ──────────────────────────────────────────────────────────────

static void build_config_btn(lv_obj_t *parent,
                              const char *icon,   // LV_SYMBOL_* or UTF-8
                              const char *label,
                              bool        is_todo,
                              lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_obj_create(parent);
    lv_obj_set_flex_grow(btn, 1);
    lv_obj_set_height(btn, LV_PCT(100));
    lv_obj_set_style_bg_color(btn, lv_color_hex(COL_STATUS_BOX_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, lv_color_hex(COL_BORDER), LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, 8, LV_PART_MAIN);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(btn, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(btn, 4, LV_PART_MAIN);

    if (cb) {
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_t *icon_lbl = lv_label_create(btn);
    lv_label_set_text(icon_lbl, icon);
    lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(0x444444), LV_PART_MAIN);

    lv_obj_t *text_lbl = lv_label_create(btn);
    lv_label_set_text(text_lbl, label);
    lv_obj_set_style_text_font(text_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(text_lbl, lv_color_hex(0x444444), LV_PART_MAIN);

    if (is_todo) {
        lv_obj_set_style_opa(btn, 160, LV_PART_MAIN); // dim to indicate not yet available
    }
}

static void build_config_strip(lv_obj_t *parent)
{
    lv_obj_t *strip = lv_obj_create(parent);
    lv_obj_set_size(strip, LV_PCT(100), CONFIG_STRIP_H);
    lv_obj_set_style_bg_color(strip, lv_color_hex(COL_LIGHT_BG), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(strip, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_side(strip, LV_BORDER_SIDE_TOP, LV_PART_MAIN);
    lv_obj_set_style_border_width(strip, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(strip, lv_color_hex(COL_BORDER), LV_PART_MAIN);
    lv_obj_set_style_radius(strip, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(strip, 14, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(strip, 6, LV_PART_MAIN);
    lv_obj_clear_flag(strip, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(strip, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(strip, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(strip, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(strip, 12, LV_PART_MAIN);

    build_config_btn(strip, LV_SYMBOL_WIFI,     "WiFi Settings",  false, on_wifi_clicked);
    build_config_btn(strip, LV_SYMBOL_SETTINGS, "LoRa Settings",  true,  NULL);
    build_config_btn(strip, LV_SYMBOL_GPS,      "Fixed Position", false, on_fixedpos_clicked);
}

// ── Public API ────────────────────────────────────────────────────────────────

lv_obj_t *screen_base_config_create(const app_theme_t *t)
{
    (void)t; // light mode colours are hardcoded in this screen

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
    build_pos_banner(scr);
    build_cards_area(scr);
    build_config_strip(scr);

    ESP_LOGI(TAG, "Base Config screen created");
    return scr;
}
