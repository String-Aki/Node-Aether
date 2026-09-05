/**
 * node_state.h
 *
 * Node-Aether — Real ESP32-S3 system state snapshot.
 * Replaces sim_mock_state.h from the PC simulator.
 *
 * Provides the same system_state_t struct and API so dashboard_ui.c
 * can be ported with only an #include swap.
 *
 * Data sources:
 *   wifi_state / wifi_ssid / rssi_dbm  — esp_wifi_sta_get_ap_info()
 *   vpn_state / vpn_display            — TODO: wire Tailscale/MicroLink
 *   relay1_on / relay2_on              — relay_get_state()
 *   core_temp_c                        — temperature_sensor_get_celsius()
 *   free_heap_kb                       — heap_caps_get_free_size(INTERNAL)
 *   free_psram_mb                      — heap_caps_get_free_size(SPIRAM)
 *   phone_battery_pct                  — web_server_get_phone_battery()
 *   log_lines                          — diag_log_get_last_n_lines()
 *   uptime_seconds                     — xTaskGetTickCount()
 */
#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ---------- VPN / MicroLink state (mirrors ml_state_t in main.c) ---------- */
typedef enum {
    ML_STATE_CONNECTED    = 0,
    ML_STATE_RECONNECTING = 1,
    ML_STATE_OFFLINE      = 2,
} ml_state_t;

/* ---------- WiFi state ----------------------------------------------------- */
typedef enum {
    WIFI_STATE_CONNECTED    = 0,
    WIFI_STATE_RECONNECTING = 1,
    WIFI_STATE_DISCONNECTED = 2,
} wifi_state_t;

/* ---------- Shared snapshot struct ---------------------------------------- */
typedef struct {
    uint32_t uptime_seconds;

    wifi_state_t  wifi_state;
    char          wifi_ssid[32];
    int8_t        wifi_rssi;

    ml_state_t    vpn_state;
    char          vpn_ip[20];

    bool          relay1_on;
    bool          relay2_on;

    float         core_temp_c;
    uint32_t      free_heap_kb;
    uint32_t      free_psram_mb;
    int8_t        rssi_dbm;
    int8_t        phone_battery_pct;
    char          vpn_display[20];

    char          log_lines[10][80];
    bool          show_crash_banner;
    char          crash_reason[48];
} system_state_t;

/* ---------- Public API ----------------------------------------------------- */

/**
 * Populate and return a fresh snapshot from real hardware sources.
 * Call every ~2 s from the LVGL timer callback (same thread as dashboard_ui).
 */
const system_state_t *node_state_tick(void);

/**
 * Return the last snapshot without re-querying hardware.
 * Safe to call at any time from the LVGL thread.
 */
const system_state_t *node_state_get(void);

/**
 * Pass in a temperature sensor handle obtained by the caller.
 * Must be called once before node_state_tick().
 */
void node_state_set_temp_sensor(void *temp_sensor_handle);
