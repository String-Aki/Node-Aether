/**
 * dashboard_ui.h
 *
 * Node-Aether — 5-screen Live Dashboard (ESP32-S3 firmware build).
 * 320×240 landscape (ST7789).
 *
 * 5 screens:
 *   0 — Network       (WiFi + VPN)
 *   1 — Relay         (R1 + R2 state)
 *   2 — System Health (Core Temp + Free Heap)
 *   3 — Memory/Power  (Free PSRAM + Phone Battery)
 *   4 — Live Log      (10-line diag_log feed)
 *
 * Build-once: widgets created once at init, only content updated per tick.
 * Ported from lv_port_pc_vscode/src/node_aether/dashboard_ui.h
 */
#pragma once

#include "lvgl.h"
#include "node_state.h"

#define DASHBOARD_SCREEN_COUNT 5

/** Build all 5 screen objects and their widgets. Call once after display_manager_init(). */
void dashboard_ui_init(void);

/** Push fresh state data into every screen's widgets. Call from the LVGL task context. */
void dashboard_ui_update(const system_state_t *state);

/**
 * Switch the visible screen with a smooth fade.
 * idx must be 0 – (DASHBOARD_SCREEN_COUNT – 1).
 */
void dashboard_ui_set_screen(int idx);

/** Returns the index of the currently visible screen. */
int dashboard_ui_get_current_screen(void);

/** Trigger a 150 ms white-flash on the relay dot (relay_index: 0=R1, 1=R2). */
void dashboard_ui_relay_flash(uint8_t relay_index);

/** lv_timer callback: advance state, update UI, handle HTTP screen-switch requests. */
void dashboard_ui_timer_cb(lv_timer_t *timer);
