#include "screen_home.h"
#include "esp_log.h"

static const char *TAG = "screen_home";

static lv_obj_t *s_nav_base = NULL;

void screen_home_set_nav_base(lv_obj_t *base_config_scr)
{
	s_nav_base = base_config_scr;
}

// ── Layout constants ──────────────────────────────────────────────────────────

#define STATUS_BAR_H    28
#define HERO_H          168
#define VERSION_STRIP_H 24
// Cards area takes all remaining height: 600 - 28 - 168 - 24 = 380

// ── Shared container helper ───────────────────────────────────────────────────
// Transparent, no border, no padding, no scroll — base for custom containers.

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

// ── Status bar ────────────────────────────────────────────────────────────────
// 28px strip at top of every screen.
// [F] battery values — placeholder until BLE + ADC live.
// [F] GPS date/time — placeholder until Rover F9P + BLE live.

static lv_obj_t *build_status_bar(lv_obj_t *parent, const app_theme_t *t)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, LV_PCT(100), STATUS_BAR_H);
    lv_obj_set_style_bg_color(bar, tc(t->bg_secondary), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(bar, 14, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(bar, 0, LV_PART_MAIN);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                               LV_FLEX_ALIGN_CENTER,
                               LV_FLEX_ALIGN_CENTER);

    // ── Left group: base + rover battery ──────────────────────────────────────
    lv_obj_t *left = make_cont(bar);
    lv_obj_set_size(left, LV_SIZE_CONTENT, STATUS_BAR_H);
    lv_obj_set_layout(left, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(left, 18, LV_PART_MAIN);

    lv_obj_t *base_batt = lv_label_create(left);
    lv_label_set_text(base_batt, "Base: --%");   // [F] replaced by BLE beacon
    lv_obj_set_style_text_font(base_batt, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(base_batt, tc(t->text_muted), LV_PART_MAIN);

    lv_obj_t *rover_batt = lv_label_create(left);
    lv_label_set_text(rover_batt, "Rover: --%"); // [F] replaced by BLE beacon
    lv_obj_set_style_text_font(rover_batt, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(rover_batt, tc(t->text_muted), LV_PART_MAIN);

    // ── Right group: GPS time + tablet battery ────────────────────────────────
    lv_obj_t *right = make_cont(bar);
    lv_obj_set_size(right, LV_SIZE_CONTENT, STATUS_BAR_H);
    lv_obj_set_layout(right, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(right, 14, LV_PART_MAIN);

    lv_obj_t *gps_time = lv_label_create(right);
    lv_label_set_text(gps_time, "----/--/-- --:--:--"); // [F] replaced by UBX-NAV-PVT
    lv_obj_set_style_text_font(gps_time, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(gps_time, lv_color_hex(0x37474F), LV_PART_MAIN);

    lv_obj_t *tablet_batt = lv_label_create(right);
    lv_label_set_text(tablet_batt, "--%");            // [F] replaced by ADC read
    lv_obj_set_style_text_font(tablet_batt, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(tablet_batt, tc(t->text_muted), LV_PART_MAIN);

    return bar;
}

// ── GNSS icon ─────────────────────────────────────────────────────────────────
// Three concentric top-half arcs + antenna dot + staff pole + ground foot.
// Approximates the SVG in 00002-HomeScreen.html using LVGL primitives.

static void build_gnss_icon(lv_obj_t *parent)
{
    // 80×68 transparent container, children positioned absolutely
    lv_obj_t *icon = lv_obj_create(parent);
    lv_obj_set_size(icon, 80, 68);
    lv_obj_set_style_bg_opa(icon, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(icon, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(icon, 0, LV_PART_MAIN);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);

    // Three signal arcs — top semicircle (180°→360° CW = via 12-o'clock).
    // Each arc widget is centred on the antenna position (40,38/40/42) and
    // sized to match the SVG radius.
    static const struct {
        int16_t   x, y, sz; // widget top-left position + side length
        lv_opa_t  opa;       // arc track opacity
    } arc_spec[] = {
        {  4,  2, 72,  89 }, // outer  (SVG r=36, centre 40,38) ~35%
        { 15, 15, 50, 153 }, // middle (SVG r=25, centre 40,40) ~60%
        { 26, 28, 28, 255 }, // inner  (SVG r=14, centre 40,42) 100%
    };
    for (int i = 0; i < 3; i++) {
        lv_obj_t *arc = lv_arc_create(icon);
        lv_obj_set_size(arc, arc_spec[i].sz, arc_spec[i].sz);
        lv_obj_set_pos(arc, arc_spec[i].x, arc_spec[i].y);

        // Top-half arc: 180° CW to 360° (left → 12-o'clock → right)
        lv_arc_set_bg_angles(arc, 180, 360);

        // Widget body: transparent
        lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(arc, 0, LV_PART_MAIN);

        // Arc track (MAIN arc properties): coloured
        lv_obj_set_style_arc_color(arc, lv_color_hex(0x2196F3), LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc, 3, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(arc, arc_spec[i].opa, LV_PART_MAIN);

        // Indicator arc: hidden
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_INDICATOR);

        // Knob: hidden
        lv_obj_set_style_opa(arc, LV_OPA_TRANSP, LV_PART_KNOB);
        lv_obj_set_style_pad_all(arc, 0, LV_PART_KNOB);
    }

    // Antenna dot — filled circle at (35,40), 10×10 px (centre 40,45)
    lv_obj_t *dot = lv_obj_create(icon);
    lv_obj_set_size(dot, 10, 10);
    lv_obj_set_pos(dot, 35, 40);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(dot, lv_color_hex(0x2196F3), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

    // Staff pole — vertical line (40,50)→(40,64)
    // lv_line points are relative to widget origin; widget positioned at (40,50)
    static const lv_point_precise_t pole_pts[] = {{0, 0}, {0, 14}};
    lv_obj_t *pole = lv_line_create(icon);
    lv_line_set_points(pole, pole_pts, 2);
    lv_obj_set_pos(pole, 40, 50);
    lv_obj_set_style_line_color(pole, lv_color_hex(0x546E7A), LV_PART_MAIN);
    lv_obj_set_style_line_width(pole, 3, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(pole, true, LV_PART_MAIN);

    // Ground foot — polyline (33,62)→(40,68)→(47,62); widget at (33,62)
    static const lv_point_precise_t foot_pts[] = {{0, 0}, {7, 6}, {14, 0}};
    lv_obj_t *foot = lv_line_create(icon);
    lv_line_set_points(foot, foot_pts, 3);
    lv_obj_set_pos(foot, 33, 62);
    lv_obj_set_style_line_color(foot, lv_color_hex(0x546E7A), LV_PART_MAIN);
    lv_obj_set_style_line_width(foot, 2, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(foot, true, LV_PART_MAIN);
}

// ── Hero section ──────────────────────────────────────────────────────────────
// 168px centred column: GNSS icon + product name + tagline.

static lv_obj_t *build_hero(lv_obj_t *parent, const app_theme_t *t)
{
    lv_obj_t *hero = lv_obj_create(parent);
    lv_obj_set_size(hero, LV_PCT(100), HERO_H);
    lv_obj_set_style_bg_opa(hero, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_side(hero, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_border_width(hero, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(hero, lv_color_hex(0x1E2533), LV_PART_MAIN);
    lv_obj_set_style_border_opa(hero, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(hero, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(hero, 0, LV_PART_MAIN);
    lv_obj_clear_flag(hero, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(hero, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hero, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(hero, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(hero, 6, LV_PART_MAIN);

    build_gnss_icon(hero);

    lv_obj_t *title = lv_label_create(hero);
    lv_label_set_text(title, "GPS Staff");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, tc(t->text_primary), LV_PART_MAIN);

    lv_obj_t *tagline = lv_label_create(hero);
    lv_label_set_text(tagline, "RTK SURVEY INSTRUMENT");
    lv_obj_set_style_text_font(tagline, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(tagline, tc(t->text_muted), LV_PART_MAIN);

    return hero;
}

// ── Card helpers ──────────────────────────────────────────────────────────────

// Add a bullet row to a flex-column bullets container.
static void add_bullet(lv_obj_t *parent,
                       const char *text,
                       lv_color_t  dot_col,
                       lv_color_t  text_col)
{
    lv_obj_t *row = make_cont(parent);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(row, 10, LV_PART_MAIN);

    lv_obj_t *dot = lv_label_create(row);
    lv_label_set_text(dot, "\xE2\x80\xA2"); // U+2022 bullet
    lv_obj_set_style_text_color(dot, dot_col, LV_PART_MAIN);
    lv_obj_set_style_text_font(dot, &lv_font_montserrat_16, LV_PART_MAIN);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_color(lbl, text_col, LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_flex_grow(lbl, 1);
}

// Connection status dot + label.
static void add_ble_dot(lv_obj_t *parent, const char *status_text, lv_color_t dot_col)
{
    lv_obj_t *row = make_cont(parent);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 6, LV_PART_MAIN);

    lv_obj_t *dot = lv_obj_create(row);
    lv_obj_set_size(dot, 8, 8);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(dot, dot_col, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
    lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, status_text);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0x546E7A), LV_PART_MAIN);
}

// ── Base card ─────────────────────────────────────────────────────────────────

static void on_base_card_clicked(lv_event_t *e)
{
	(void)e;
	ESP_LOGI(TAG, "Base card tapped → navigate to Base Config");
	if (s_nav_base) lv_scr_load(s_nav_base);
}

static lv_obj_t *build_base_card(lv_obj_t *parent, const app_theme_t *t)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_flex_grow(card, 1);
    lv_obj_set_height(card, LV_PCT(100)); // fill area height (no STRETCH in LV flex)
    lv_obj_set_style_bg_color(card, tc(t->bg_secondary), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
    lv_obj_set_style_border_side(card, LV_BORDER_SIDE_LEFT, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, tc(t->accent), LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_left(card, 22, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(card, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(card, on_base_card_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(card, LV_OBJ_FLAG_CLICKABLE);

    // ── Header row ────────────────────────────────────────────────────────────
    lv_obj_t *hdr = make_cont(card);
    lv_obj_set_width(hdr, LV_PCT(100));
    lv_obj_set_height(hdr, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_bottom(hdr, 14, LV_PART_MAIN);
    lv_obj_set_layout(hdr, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(hdr, 14, LV_PART_MAIN);

    // Icon badge
    lv_obj_t *badge = lv_obj_create(hdr);
    lv_obj_set_size(badge, 44, 44);
    lv_obj_set_style_radius(badge, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge, lv_color_hex(0x0D47A1), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(badge, 0, LV_PART_MAIN);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *badge_lbl = lv_label_create(badge);
    lv_label_set_text(badge_lbl, "\xE2\x8A\x99"); // U+2299 ⊙
    lv_obj_set_style_text_color(badge_lbl, lv_color_hex(0x90CAF9), LV_PART_MAIN);
    lv_obj_set_style_text_font(badge_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(badge_lbl);

    // Title + subtitle column
    lv_obj_t *title_col = make_cont(hdr);
    lv_obj_set_flex_grow(title_col, 1);
    lv_obj_set_height(title_col, LV_SIZE_CONTENT);
    lv_obj_set_layout(title_col, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(title_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(title_col, 2, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(title_col);
    lv_label_set_text(title, "Base Station");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, tc(t->text_primary), LV_PART_MAIN);

    lv_obj_t *sub = lv_label_create(title_col);
    lv_label_set_text(sub, "Fixed position \xC2\xB7 RTCM broadcast"); // · U+00B7
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(sub, tc(t->text_muted), LV_PART_MAIN);

    // BLE dot — grey (never connected) [F] until BLE live
    add_ble_dot(hdr, "---", lv_color_hex(0x546E7A));

    // ── Bullets ───────────────────────────────────────────────────────────────
    lv_obj_t *bullets = make_cont(card);
    lv_obj_set_width(bullets, LV_PCT(100));
    lv_obj_set_flex_grow(bullets, 1);
    lv_obj_set_layout(bullets, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bullets, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(bullets, 8, LV_PART_MAIN);

    add_bullet(bullets,
               "Survey-in position or enter a known monument",
               tc(t->accent), lv_color_hex(0x78909C));
    add_bullet(bullets,
               "Broadcast RTCM corrections over GFSK radio",
               tc(t->accent), lv_color_hex(0x78909C));
    add_bullet(bullets,
               "WiFi NTRIP gateway to OS Net correction service",
               tc(t->accent), lv_color_hex(0x78909C));

    // ── Button ────────────────────────────────────────────────────────────────
    lv_obj_t *btn = lv_button_create(card);
    lv_obj_set_size(btn, LV_PCT(100), 48);
    lv_obj_set_style_margin_top(btn, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1565C0), LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, on_base_card_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, "Configure Base  " LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn_lbl, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(btn_lbl);

    return card;
}

// ── Rover card (disabled) ─────────────────────────────────────────────────────

static lv_obj_t *build_rover_card(lv_obj_t *parent, const app_theme_t *t)
{
    (void)t;

    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_flex_grow(card, 1);
    lv_obj_set_height(card, LV_PCT(100)); // fill area height (no STRETCH in LV flex)
    lv_obj_set_style_bg_color(card, lv_color_hex(0x181F28), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 12, LV_PART_MAIN);
    lv_obj_set_style_border_side(card, LV_BORDER_SIDE_LEFT, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 4, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(0x37474F), LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_left(card, 22, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_layout(card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(card, 0, LV_PART_MAIN);

    // Dim the whole card (~55% opacity)
    lv_obj_set_style_opa(card, 140, LV_PART_MAIN);

    // ── Header row ────────────────────────────────────────────────────────────
    lv_obj_t *hdr = make_cont(card);
    lv_obj_set_width(hdr, LV_PCT(100));
    lv_obj_set_height(hdr, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_bottom(hdr, 14, LV_PART_MAIN);
    lv_obj_set_layout(hdr, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(hdr, 14, LV_PART_MAIN);

    // Icon badge
    lv_obj_t *badge = lv_obj_create(hdr);
    lv_obj_set_size(badge, 44, 44);
    lv_obj_set_style_radius(badge, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge, lv_color_hex(0x1C2A35), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(badge, 0, LV_PART_MAIN);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *badge_lbl = lv_label_create(badge);
    lv_label_set_text(badge_lbl, "\xE2\x8A\x9B"); // U+229B ⊛
    lv_obj_set_style_text_color(badge_lbl, lv_color_hex(0x546E7A), LV_PART_MAIN);
    lv_obj_set_style_text_font(badge_lbl, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_center(badge_lbl);

    // Title + subtitle
    lv_obj_t *title_col = make_cont(hdr);
    lv_obj_set_flex_grow(title_col, 1);
    lv_obj_set_height(title_col, LV_SIZE_CONTENT);
    lv_obj_set_layout(title_col, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(title_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(title_col, 2, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(title_col);
    lv_label_set_text(title, "Rover");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, lv_color_hex(0x78909C), LV_PART_MAIN);

    lv_obj_t *sub = lv_label_create(title_col);
    lv_label_set_text(sub, "Mobile receiver \xC2\xB7 RTK fix");
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(sub, lv_color_hex(0x37474F), LV_PART_MAIN);

    // "Coming soon" badge
    lv_obj_t *badge2 = lv_obj_create(hdr);
    lv_obj_set_size(badge2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(badge2, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge2, lv_color_hex(0x1C2A35), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge2, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(badge2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(badge2, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(badge2, 3, LV_PART_MAIN);
    lv_obj_clear_flag(badge2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *soon_lbl = lv_label_create(badge2);
    lv_label_set_text(soon_lbl, "Coming soon");
    lv_obj_set_style_text_font(soon_lbl, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(soon_lbl, lv_color_hex(0x455A64), LV_PART_MAIN);
    lv_obj_center(soon_lbl);

    // ── Bullets ───────────────────────────────────────────────────────────────
    lv_obj_t *bullets = make_cont(card);
    lv_obj_set_width(bullets, LV_PCT(100));
    lv_obj_set_flex_grow(bullets, 1);
    lv_obj_set_layout(bullets, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bullets, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(bullets, 8, LV_PART_MAIN);

    lv_color_t dim = lv_color_hex(0x37474F);
    add_bullet(bullets, "Receive RTCM corrections from Base over GFSK", dim, dim);
    add_bullet(bullets, "Achieve RTK fixed solution \xE2\x80\x94 centimetre accuracy", dim, dim);
    add_bullet(bullets, "Log survey points and export to standard formats", dim, dim);

    // ── Disabled button ───────────────────────────────────────────────────────
    lv_obj_t *btn = lv_button_create(card);
    lv_obj_set_size(btn, LV_PCT(100), 48);
    lv_obj_set_style_margin_top(btn, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1C2A35), LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *btn_lbl = lv_label_create(btn);
    lv_label_set_text(btn_lbl, "Not yet available");
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn_lbl, lv_color_hex(0x37474F), LV_PART_MAIN);
    lv_obj_center(btn_lbl);

    return card;
}

// ── Cards area ────────────────────────────────────────────────────────────────

static lv_obj_t *build_cards(lv_obj_t *parent, const app_theme_t *t)
{
    lv_obj_t *area = make_cont(parent);
    lv_obj_set_width(area, LV_PCT(100));
    lv_obj_set_flex_grow(area, 1); // fill remaining height
    lv_obj_set_layout(area, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(area, LV_FLEX_FLOW_ROW);
    // LV_FLEX_ALIGN_STRETCH not available in LVGL 9; cards fill height via LV_PCT(100)
    lv_obj_set_flex_align(area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_left(area, 40, LV_PART_MAIN);
    lv_obj_set_style_pad_right(area, 40, LV_PART_MAIN);
    lv_obj_set_style_pad_top(area, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(area, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_column(area, 20, LV_PART_MAIN);

    build_base_card(area, t);
    build_rover_card(area, t);

    return area;
}

// ── Version strip ─────────────────────────────────────────────────────────────

static lv_obj_t *build_version_strip(lv_obj_t *parent, const app_theme_t *t)
{
    (void)t;

    lv_obj_t *strip = lv_obj_create(parent);
    lv_obj_set_size(strip, LV_PCT(100), VERSION_STRIP_H);
    lv_obj_set_style_bg_color(strip, lv_color_hex(0x0D1520), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(strip, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(strip, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(strip, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(strip, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(strip, 0, LV_PART_MAIN);
    lv_obj_clear_flag(strip, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *ver = lv_label_create(strip);
    // \xC2\xB7 = UTF-8 for U+00B7 MIDDLE DOT (·)
    lv_label_set_text(ver, "GPS STAFF  \xC2\xB7  PCB v1.0  \xC2\xB7  fw 0.1.0");
    lv_obj_set_style_text_font(ver, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_color(ver, lv_color_hex(0x263238), LV_PART_MAIN);
    lv_obj_align(ver, LV_ALIGN_RIGHT_MID, 0, 0);

    return strip;
}

// ── Public API ────────────────────────────────────────────────────────────────

lv_obj_t *screen_home_create(const app_theme_t *t)
{
    lv_obj_t *scr = lv_obj_create(NULL); // NULL parent = new screen object
    lv_obj_set_size(scr, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(scr, tc(t->bg_primary), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(scr, 0, LV_PART_MAIN);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(scr, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(scr, 0, LV_PART_MAIN);

    build_status_bar(scr, t);
    build_hero(scr, t);
    build_cards(scr, t);
    build_version_strip(scr, t);

    ESP_LOGI(TAG, "Home screen created");
    return scr;
}
