/**
 * dashboard_ui.c
 *
 * Node-Aether — 5-screen Live Dashboard (ESP32-S3 firmware build).
 * Display: 320×240 landscape (ST7789).
 *
 * Ported from lv_port_pc_vscode/src/node_aether/dashboard_ui.c.
 * Only changes:
 *   - #include "sim_mock_state.h"   → #include "node_state.h"
 *   - #include "sim_http_server.h"  → #include "display_controller.h"
 *   - #include "ui_fonts.h"         → #include "fonts/ui_fonts.h"
 *   - sim_mock_state_get()          → node_state_get()
 *   - sim_mock_state_tick()         → node_state_tick()
 *   - sim_http_server_*()           → display_controller_*()
 *
 * All UI logic, widget layout, animations, and color palette are unchanged.
 */

#include "dashboard_ui.h"
#include "node_state.h"
#include "display_controller.h"
#include "fonts/ui_fonts.h"

#include <stdio.h>
#include <string.h>

/* ============================================================================
 * COLOR PALETTE  (PRD §3 — semantic constants, chosen for maximum vibrancy
 *                on a dark TFT; all near-exact RGB565 values)
 * ========================================================================== */
#define CLR_BG_BASE     lv_color_hex(0x060C14)   /* deep near-black navy */
#define CLR_PANEL_FILL  lv_color_hex(0x0C1A2E)   /* deep navy-blue card fill */
#define CLR_HEADER_BAR  lv_color_hex(0x00BFFF)   /* electric sky-blue */
#define CLR_GOOD        lv_color_hex(0x00FF7F)   /* neon spring-green */
#define CLR_INFO        lv_color_hex(0x00E5FF)   /* electric cyan */
#define CLR_WARN        lv_color_hex(0xFFAA00)   /* electric amber */
#define CLR_BAD         lv_color_hex(0xFF2D55)   /* hot crimson-red */
#define CLR_MUTED       lv_color_hex(0x6080A0)   /* raised slate */
#define CLR_BORDER      lv_color_hex(0x1A3A5C)   /* blue-tinted hairlines */
#define CLR_WHITE       lv_color_hex(0xFFFFFF)

/* ============================================================================
 * LAYOUT CONSTANTS
 * ========================================================================== */
#define DISPLAY_W   320
#define DISPLAY_H   240
#define HEADER_H    32
#define CONTENT_Y   HEADER_H
#define CONTENT_H   (DISPLAY_H - HEADER_H)   /* 208 px */
#define HALF_H      (CONTENT_H / 2)          /* 104 px */
#define HALF_Y2     (CONTENT_Y + HALF_H)     /* 136 */

/* ============================================================================
 * SCREEN REGISTRY
 * ========================================================================== */
#define SCREEN_COUNT DASHBOARD_SCREEN_COUNT

static lv_obj_t *s_screens[SCREEN_COUNT];
static int        s_current = 0;
static uint32_t   s_slide_elapsed_ms = 0;

#define TICK_MS 2000

static const char *SCREEN_TITLES[SCREEN_COUNT] = {
    "NETWORK", "RELAY STATUS", "SYSTEM HEALTH", "MEM & POWER", "LIVE LOG"
};

/* ============================================================================
 * SHARED WIDGET HANDLES
 * ========================================================================== */

static lv_obj_t *s_uptime_lbl[SCREEN_COUNT];

/* --- Screen 0: Network --- */
static lv_obj_t *s0_wifi_ssid;
static lv_obj_t *s0_wifi_rssi;
static lv_obj_t *s0_wifi_state;
static lv_obj_t *s0_vpn_state;
static lv_obj_t *s0_vpn_ip;

static lv_anim_t s_vpn_anim;
static bool      s_vpn_pulsing = false;
static lv_obj_t *s0_vpn_state_bg;

/* --- Screen 1: Relay --- */
static lv_obj_t *s1_r1_circle;
static lv_obj_t *s1_r1_state;
static lv_obj_t *s1_r2_circle;
static lv_obj_t *s1_r2_state;

typedef struct { lv_obj_t *circle; lv_color_t final_color; } relay_ctx_t;
static relay_ctx_t s_relay_ctx[2];

/* --- Screen 2: System Health --- */
static lv_obj_t *s2_temp_val;
static lv_obj_t *s2_heap_val;

/* --- Screen 3: Memory & Power --- */
static lv_obj_t *s3_psram_val;
static lv_obj_t *s3_batt_val;

/* --- Screen 4: Live Log --- */
#define LOG_SCREEN_LINES 10
static lv_obj_t *s4_lines[LOG_SCREEN_LINES];

/* ============================================================================
 * STYLE HELPERS
 * ========================================================================== */

static lv_obj_t *make_label(lv_obj_t *parent,
                             const lv_font_t *font,
                             lv_color_t color,
                             int32_t w)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    lv_obj_set_style_bg_opa(lbl, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(lbl, 0, 0);
    lv_obj_set_style_pad_all(lbl, 0, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
    if (w > 0) lv_obj_set_width(lbl, w);
    return lbl;
}

static void flat_bg(lv_obj_t *obj, lv_color_t color) {
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
}

static void add_divider(lv_obj_t *screen, int32_t y) {
    lv_obj_t *div = lv_obj_create(screen);
    lv_obj_set_pos(div, 0, y);
    lv_obj_set_size(div, DISPLAY_W, 1);
    flat_bg(div, CLR_BORDER);
}

static void add_vdivider(lv_obj_t *screen, int32_t x, int32_t y1, int32_t y2) {
    lv_obj_t *div = lv_obj_create(screen);
    lv_obj_set_pos(div, x, y1);
    lv_obj_set_size(div, 1, y2 - y1);
    flat_bg(div, CLR_BORDER);
}

static lv_obj_t *section_key(lv_obj_t *screen, const char *text,
                              int32_t x, int32_t y, int32_t w) {
    lv_obj_t *lbl = make_label(screen, &ui_font_share_tech_mono_12, CLR_MUTED, w);
    lv_label_set_text(lbl, text);
    lv_obj_set_pos(lbl, x, y);
    return lbl;
}

/* ============================================================================
 * STANDARD SCREEN HEADER
 * ========================================================================== */

static lv_obj_t *create_header(lv_obj_t *screen, const char *title) {
    lv_obj_t *bar = lv_obj_create(screen);
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, DISPLAY_W, HEADER_H);
    flat_bg(bar, CLR_HEADER_BAR);
    lv_obj_set_style_pad_left(bar, 8, 0);
    lv_obj_set_style_pad_right(bar, 8, 0);

    lv_obj_t *title_lbl = make_label(bar, &ui_font_orbitron_12, CLR_BG_BASE, 180);
    lv_label_set_text(title_lbl, title);
    lv_obj_align(title_lbl, LV_ALIGN_LEFT_MID, 0, 0);

    lv_obj_t *uptime = make_label(bar, &ui_font_share_tech_mono_12, CLR_BG_BASE, 110);
    lv_label_set_text(uptime, "0d 00h 00m");
    lv_obj_align(uptime, LV_ALIGN_RIGHT_MID, 0, 0);

    return uptime;
}

static void update_uptime(const system_state_t *st) {
    uint32_t s    = st->uptime_seconds;
    uint32_t days = s / 86400;
    uint32_t hrs  = (s % 86400) / 3600;
    uint32_t mins = (s % 3600)  / 60;
    char buf[20];
    snprintf(buf, sizeof(buf), "%ud %02uh %02um",
             (unsigned)days, (unsigned)hrs, (unsigned)mins);
    for (int i = 0; i < SCREEN_COUNT; i++) {
        lv_label_set_text(s_uptime_lbl[i], buf);
    }
}

/* ============================================================================
 * STATE → COLOR HELPERS
 * ========================================================================== */

static lv_color_t wifi_color(wifi_state_t s) {
    switch (s) {
        case WIFI_STATE_CONNECTED:    return CLR_GOOD;
        case WIFI_STATE_RECONNECTING: return CLR_WARN;
        default:                      return CLR_BAD;
    }
}

static lv_color_t vpn_color(ml_state_t s) {
    switch (s) {
        case ML_STATE_CONNECTED:    return CLR_GOOD;
        case ML_STATE_RECONNECTING: return CLR_WARN;
        default:                    return CLR_BAD;
    }
}

static lv_color_t temp_color(float t) {
    if (t < 60.0f) return CLR_GOOD;
    if (t < 80.0f) return CLR_WARN;
    return CLR_BAD;
}

static lv_color_t batt_color(int8_t pct) {
    if (pct < 0)   return CLR_MUTED;
    if (pct < 20)  return CLR_BAD;
    if (pct < 50)  return CLR_WARN;
    return CLR_GOOD;
}

/* ============================================================================
 * SCREEN 0 — NETWORK  (WiFi + VPN)
 * ========================================================================== */

static void screen0_create(void) {
    lv_obj_t *scr = s_screens[0];
    flat_bg(scr, CLR_BG_BASE);

    s_uptime_lbl[0] = create_header(scr, SCREEN_TITLES[0]);

    section_key(scr, "WiFi", 16, 40, 150);

    s0_wifi_ssid = make_label(scr, &ui_font_orbitron_20, CLR_GOOD, DISPLAY_W - 32);
    lv_obj_set_pos(s0_wifi_ssid, 16, 62);

    s0_wifi_rssi = make_label(scr, &ui_font_share_tech_mono_20, CLR_INFO, 200);
    lv_obj_set_pos(s0_wifi_rssi, 16, 90);

    s0_wifi_state = make_label(scr, &ui_font_share_tech_mono_12, CLR_MUTED, DISPLAY_W - 32);
    lv_obj_set_pos(s0_wifi_state, 16, 116);

    add_divider(scr, 135);

    section_key(scr, "VPN / TAILSCALE", 16, 144, 200);

    s0_vpn_state_bg = lv_obj_create(scr);
    lv_obj_set_pos(s0_vpn_state_bg, 12, 164);
    lv_obj_set_size(s0_vpn_state_bg, DISPLAY_W - 24, 28);
    lv_obj_set_style_bg_color(s0_vpn_state_bg, CLR_PANEL_FILL, 0);
    lv_obj_set_style_bg_opa(s0_vpn_state_bg, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s0_vpn_state_bg, 0, 0);
    lv_obj_set_style_radius(s0_vpn_state_bg, 4, 0);
    lv_obj_set_style_pad_all(s0_vpn_state_bg, 0, 0);

    s0_vpn_state = make_label(s0_vpn_state_bg, &ui_font_orbitron_20, CLR_GOOD, DISPLAY_W - 36);
    lv_obj_align(s0_vpn_state, LV_ALIGN_LEFT_MID, 8, 0);

    s0_vpn_ip = make_label(scr, &ui_font_share_tech_mono_20, CLR_INFO, 280);
    lv_obj_set_pos(s0_vpn_ip, 16, 200);
}

static void vpn_opa_cb(void *var, int32_t val) {
    lv_obj_set_style_bg_opa((lv_obj_t *)var, (lv_opa_t)val, 0);
}

static void screen0_update(const system_state_t *st) {
    lv_color_t wc = wifi_color(st->wifi_state);
    lv_obj_set_style_text_color(s0_wifi_ssid, wc, 0);
    if (st->wifi_state == WIFI_STATE_CONNECTED) {
        lv_label_set_text(s0_wifi_ssid,  st->wifi_ssid[0] ? st->wifi_ssid : "--");
        lv_label_set_text(s0_wifi_state, "CONNECTED");
    } else if (st->wifi_state == WIFI_STATE_RECONNECTING) {
        lv_label_set_text(s0_wifi_ssid,  "RECONNECTING...");
        lv_label_set_text(s0_wifi_state, "");
    } else {
        lv_label_set_text(s0_wifi_ssid,  "DISCONNECTED");
        lv_label_set_text(s0_wifi_state, "");
    }

    char buf[24];
    snprintf(buf, sizeof(buf), "%d dBm", (int)st->rssi_dbm);
    lv_label_set_text(s0_wifi_rssi, buf);

    lv_color_t vc = vpn_color(st->vpn_state);
    lv_obj_set_style_text_color(s0_vpn_state, vc, 0);
    const char *vstate_str;
    switch (st->vpn_state) {
        case ML_STATE_CONNECTED:    vstate_str = "CONNECTED";  break;
        case ML_STATE_RECONNECTING: vstate_str = "HANDSHAKE";  break;
        default:                    vstate_str = "OFFLINE";     break;
    }
    lv_label_set_text(s0_vpn_state, vstate_str);
    lv_label_set_text(s0_vpn_ip, st->vpn_display[0] ? st->vpn_display : "--");

    if (st->vpn_state == ML_STATE_RECONNECTING && !s_vpn_pulsing) {
        lv_anim_init(&s_vpn_anim);
        lv_anim_set_var(&s_vpn_anim, s0_vpn_state_bg);
        lv_anim_set_exec_cb(&s_vpn_anim, vpn_opa_cb);
        lv_anim_set_values(&s_vpn_anim, LV_OPA_30, LV_OPA_COVER);
        lv_anim_set_time(&s_vpn_anim, 750);
        lv_anim_set_playback_time(&s_vpn_anim, 750);
        lv_anim_set_repeat_count(&s_vpn_anim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&s_vpn_anim);
        s_vpn_pulsing = true;
    } else if (st->vpn_state != ML_STATE_RECONNECTING && s_vpn_pulsing) {
        lv_anim_del(s0_vpn_state_bg, NULL);
        lv_obj_set_style_bg_opa(s0_vpn_state_bg, LV_OPA_COVER, 0);
        s_vpn_pulsing = false;
    }
}

/* ============================================================================
 * SCREEN 1 — RELAY STATUS  (R1 left | R2 right)
 * ========================================================================== */

#define COL_W      160
#define CIRCLE_SZ   80
#define CIRCLE_X1   ((COL_W - CIRCLE_SZ) / 2)
#define CIRCLE_X2   (COL_W + CIRCLE_X1)
#define CIRCLE_Y    (CONTENT_Y + (CONTENT_H - CIRCLE_SZ) / 2)

static lv_obj_t *make_relay_column(lv_obj_t *scr, int32_t x_offset,
                                    const char *label_str,
                                    lv_obj_t **out_circle, lv_obj_t **out_state)
{
    lv_obj_t *lbl = make_label(scr, &ui_font_share_tech_mono_16, CLR_MUTED, COL_W - 16);
    lv_label_set_text(lbl, label_str);
    lv_obj_set_pos(lbl, x_offset + 8, CONTENT_Y + 12);

    lv_obj_t *circle = lv_obj_create(scr);
    lv_obj_set_pos(circle, x_offset + CIRCLE_X1, CIRCLE_Y);
    lv_obj_set_size(circle, CIRCLE_SZ, CIRCLE_SZ);
    lv_obj_set_style_radius(circle, CIRCLE_SZ / 2, 0);
    lv_obj_set_style_bg_color(circle, CLR_GOOD, 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 3, 0);
    lv_obj_set_style_border_color(circle, CLR_MUTED, 0);
    lv_obj_set_style_pad_all(circle, 0, 0);
    *out_circle = circle;

    lv_obj_t *state_lbl = make_label(scr, &ui_font_orbitron_24, CLR_GOOD, COL_W);
    lv_obj_set_pos(state_lbl, x_offset, CIRCLE_Y + CIRCLE_SZ + 14);
    lv_obj_set_style_text_align(state_lbl, LV_TEXT_ALIGN_CENTER, 0);
    *out_state = state_lbl;

    return lbl;
}

static void screen1_create(void) {
    lv_obj_t *scr = s_screens[1];
    flat_bg(scr, CLR_BG_BASE);

    s_uptime_lbl[1] = create_header(scr, SCREEN_TITLES[1]);
    add_vdivider(scr, COL_W - 1, CONTENT_Y, DISPLAY_H);

    make_relay_column(scr, 0,     "RELAY 1", &s1_r1_circle, &s1_r1_state);
    make_relay_column(scr, COL_W, "RELAY 2", &s1_r2_circle, &s1_r2_state);
}

static void set_relay_visual(lv_obj_t *circle, lv_obj_t *state_lbl, bool on) {
    lv_color_t c = on ? CLR_GOOD : CLR_MUTED;
    lv_obj_set_style_bg_color(circle, c, 0);
    lv_obj_set_style_text_color(state_lbl, c, 0);
    lv_label_set_text(state_lbl, on ? "ON" : "OFF");
}

static void screen1_update(const system_state_t *st) {
    set_relay_visual(s1_r1_circle, s1_r1_state, st->relay1_on);
    set_relay_visual(s1_r2_circle, s1_r2_state, st->relay2_on);
}

/* ============================================================================
 * SCREEN 2 — SYSTEM HEALTH  (Core Temp | Free Heap)
 * ========================================================================== */

static lv_obj_t *make_metric_section(lv_obj_t *scr, int32_t y,
                                      const char *key_text)
{
    section_key(scr, key_text, 20, y + 10, DISPLAY_W - 40);
    lv_obj_t *val = make_label(scr, &ui_font_orbitron_24, CLR_INFO, DISPLAY_W - 40);
    lv_obj_set_pos(val, 20, y + 38);
    return val;
}

static void screen2_create(void) {
    lv_obj_t *scr = s_screens[2];
    flat_bg(scr, CLR_BG_BASE);

    s_uptime_lbl[2] = create_header(scr, SCREEN_TITLES[2]);
    add_divider(scr, HALF_Y2);

    s2_temp_val = make_metric_section(scr, CONTENT_Y, "CORE TEMP");
    s2_heap_val = make_metric_section(scr, HALF_Y2,   "FREE HEAP");
}

static void screen2_update(const system_state_t *st) {
    char buf[24];

    snprintf(buf, sizeof(buf), "%.1f C", st->core_temp_c);
    lv_label_set_text(s2_temp_val, buf);
    lv_obj_set_style_text_color(s2_temp_val, temp_color(st->core_temp_c), 0);

    if (st->free_heap_kb > 0) snprintf(buf, sizeof(buf), "%u KB", (unsigned)st->free_heap_kb);
    else                       snprintf(buf, sizeof(buf), "--");
    lv_label_set_text(s2_heap_val, buf);
    lv_obj_set_style_text_color(s2_heap_val, CLR_INFO, 0);
}

/* ============================================================================
 * SCREEN 3 — MEMORY & POWER  (Free PSRAM | Phone Battery)
 * ========================================================================== */

static lv_obj_t *s3_batt_key;

static void screen3_create(void) {
    lv_obj_t *scr = s_screens[3];
    flat_bg(scr, CLR_BG_BASE);

    s_uptime_lbl[3] = create_header(scr, SCREEN_TITLES[3]);
    add_divider(scr, HALF_Y2);

    s3_psram_val = make_metric_section(scr, CONTENT_Y, "FREE PSRAM");
    s3_batt_val  = make_metric_section(scr, HALF_Y2,   "PHONE BATTERY");

    s3_batt_key = lv_obj_get_child(s_screens[3],
                                    lv_obj_get_child_cnt(s_screens[3]) - 2);
}

static void screen3_update(const system_state_t *st) {
    char buf[24];

    if (st->free_psram_mb > 0) snprintf(buf, sizeof(buf), "%u MB", (unsigned)st->free_psram_mb);
    else                        snprintf(buf, sizeof(buf), "--");
    lv_label_set_text(s3_psram_val, buf);
    lv_obj_set_style_text_color(s3_psram_val, CLR_INFO, 0);

    if (st->phone_battery_pct < 0) {
        lv_label_set_text(s3_batt_val, "--");
        lv_obj_set_style_text_color(s3_batt_val, CLR_MUTED, 0);
    } else {
        snprintf(buf, sizeof(buf), "%d%%", (int)st->phone_battery_pct);
        lv_label_set_text(s3_batt_val, buf);
        lv_obj_set_style_text_color(s3_batt_val, batt_color(st->phone_battery_pct), 0);
    }
}

/* ============================================================================
 * SCREEN 4 — LIVE LOG  (10 lines, VT323 20, newest at bottom)
 * ========================================================================== */

#define LOG_LINE_H  20

static void screen4_create(void) {
    lv_obj_t *scr = s_screens[4];
    flat_bg(scr, CLR_BG_BASE);

    s_uptime_lbl[4] = create_header(scr, SCREEN_TITLES[4]);

    for (int i = 0; i < LOG_SCREEN_LINES; i++) {
        lv_obj_t *line = make_label(scr, &ui_font_vt323_20, CLR_MUTED, DISPLAY_W - 8);
        lv_obj_set_pos(line, 4, CONTENT_Y + i * LOG_LINE_H);
        lv_label_set_text(line, "");
        s4_lines[i] = line;
    }
}

static void log_opa_cb(void *var, int32_t val) {
    lv_obj_set_style_text_opa((lv_obj_t *)var, (lv_opa_t)val, 0);
}

static void screen4_update(const system_state_t *st) {
    const char *newest  = st->log_lines[9];
    const char *current = lv_label_get_text(s4_lines[LOG_SCREEN_LINES - 1]);

    if (strcmp(newest, current) == 0) return;

    for (int i = 0; i < LOG_SCREEN_LINES - 1; i++) {
        lv_label_set_text(s4_lines[i], st->log_lines[i]);
        lv_obj_set_style_text_opa(s4_lines[i], LV_OPA_COVER, 0);
        lv_opa_t age_opa = (lv_opa_t)(LV_OPA_40 + (i * (LV_OPA_COVER - LV_OPA_40)) / (LOG_SCREEN_LINES - 1));
        lv_obj_set_style_text_opa(s4_lines[i], age_opa, 0);
    }

    lv_label_set_text(s4_lines[LOG_SCREEN_LINES - 1], newest);
    lv_obj_set_style_text_color(s4_lines[LOG_SCREEN_LINES - 1], CLR_INFO, 0);
    lv_obj_set_style_text_opa(s4_lines[LOG_SCREEN_LINES - 1], LV_OPA_TRANSP, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s4_lines[LOG_SCREEN_LINES - 1]);
    lv_anim_set_exec_cb(&a, log_opa_cb);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&a, 200);
    lv_anim_set_repeat_count(&a, 1);
    lv_anim_start(&a);

    for (int i = 0; i < LOG_SCREEN_LINES - 1; i++) {
        lv_obj_set_style_text_color(s4_lines[i], CLR_MUTED, 0);
    }
}

/* ============================================================================
 * RELAY FLASH ANIMATION  (150 ms white flash)
 * ========================================================================== */

static void relay_flash_cb(void *var, int32_t val) {
    lv_obj_t *circle = (lv_obj_t *)var;
    uint8_t idx = (circle == s1_r1_circle) ? 0 : 1;
    lv_color_t c = lv_color_mix(CLR_WHITE, s_relay_ctx[idx].final_color, (lv_opa_t)val);
    lv_obj_set_style_bg_color(circle, c, 0);
}

void dashboard_ui_relay_flash(uint8_t relay_index) {
    lv_obj_t *circle = (relay_index == 0) ? s1_r1_circle : s1_r2_circle;
    const system_state_t *st = node_state_get();
    bool on = (relay_index == 0) ? st->relay1_on : st->relay2_on;
    s_relay_ctx[relay_index].circle      = circle;
    s_relay_ctx[relay_index].final_color = on ? CLR_GOOD : CLR_MUTED;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, circle);
    lv_anim_set_exec_cb(&a, relay_flash_cb);
    lv_anim_set_values(&a, 255, 0);
    lv_anim_set_time(&a, 150);
    lv_anim_set_repeat_count(&a, 1);
    lv_anim_start(&a);
}

/* ============================================================================
 * PUBLIC API
 * ========================================================================== */

void dashboard_ui_init(void) {
    for (int i = 0; i < SCREEN_COUNT; i++) {
        s_screens[i] = lv_obj_create(NULL);
    }

    screen0_create();
    screen1_create();
    screen2_create();
    screen3_create();
    screen4_create();

    const system_state_t *st = node_state_get();
    screen0_update(st);
    screen1_update(st);
    screen2_update(st);
    screen3_update(st);
    screen4_update(st);
    update_uptime(st);

    s_current = 0;
    lv_scr_load(s_screens[0]);
    display_controller_set_current_screen(0);
}

void dashboard_ui_set_screen(int idx) {
    if (idx < 0 || idx >= SCREEN_COUNT) return;
    if (idx == s_current) return;

    if (s_current == 0 && s_vpn_pulsing) {
        lv_anim_del(s0_vpn_state_bg, NULL);
        lv_obj_set_style_bg_opa(s0_vpn_state_bg, LV_OPA_COVER, 0);
        s_vpn_pulsing = false;
    }

    s_current = idx;
    lv_scr_load_anim(s_screens[idx], LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    display_controller_set_current_screen(idx);
}

int dashboard_ui_get_current_screen(void) {
    return s_current;
}

void dashboard_ui_update(const system_state_t *state) {
    screen0_update(state);
    screen1_update(state);
    screen2_update(state);
    screen3_update(state);
    screen4_update(state);
    update_uptime(state);
}

void dashboard_ui_timer_cb(lv_timer_t *timer) {
    (void)timer;

    /* 1. HTTP screen-switch request (overrides auto-slide) */
    int req = display_controller_poll_screen_request();
    if (req >= 0) {
        s_slide_elapsed_ms = 0;
        dashboard_ui_set_screen(req);
    }

    /* 2. Auto-slide */
    bool slide_on;
    int  slide_interval_ms;
    display_controller_get_slide_config(&slide_on, &slide_interval_ms);
    if (slide_on) {
        s_slide_elapsed_ms += TICK_MS;
        if (s_slide_elapsed_ms >= (uint32_t)slide_interval_ms) {
            s_slide_elapsed_ms = 0;
            dashboard_ui_set_screen((s_current + 1) % SCREEN_COUNT);
        }
    } else {
        s_slide_elapsed_ms = 0;
    }

    /* 3. Advance real state and push to all screens */
    const system_state_t *st = node_state_tick();
    dashboard_ui_update(st);
}
