/**
 * node_state.c
 *
 * Populates system_state_t from real ESP-IDF hardware APIs.
 * Replaces sim_mock_state.c from the PC simulator.
 */
#include "node_state.h"
#include "relay.h"
#include "web_server.h"
#include "diag_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_heap_caps.h"
#include "driver/temperature_sensor.h"
#include "esp_log.h"

#include <string.h>
#include <stdio.h>

static const char *TAG = "NODE_STATE";

static system_state_t s_state;
static temperature_sensor_handle_t s_temp_sensor = NULL;

void node_state_set_temp_sensor(void *handle) {
    s_temp_sensor = (temperature_sensor_handle_t)handle;
}

const system_state_t *node_state_tick(void) {
    memset(&s_state, 0, sizeof(s_state));

    /* ── Uptime ─────────────────────────────────────────────────────────── */
    s_state.uptime_seconds =
        (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);

    /* ── WiFi ───────────────────────────────────────────────────────────── */
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        s_state.wifi_state = WIFI_STATE_CONNECTED;
        strncpy(s_state.wifi_ssid, (char *)ap_info.ssid,
                sizeof(s_state.wifi_ssid) - 1);
        s_state.wifi_rssi = (int8_t)ap_info.rssi;
        s_state.rssi_dbm  = s_state.wifi_rssi;
    } else {
        /* No AP record — could be disconnected or mid-reconnect */
        s_state.wifi_state = WIFI_STATE_RECONNECTING;
        strncpy(s_state.wifi_ssid, "", sizeof(s_state.wifi_ssid) - 1);
        s_state.wifi_rssi = 0;
        s_state.rssi_dbm  = 0;
    }

    /* ── VPN / Tailscale ────────────────────────────────────────────────
     * TODO: wire real Tailscale/MicroLink state when integrated.
     *       For now we report OFFLINE so the display shows correct state. */
    s_state.vpn_state = ML_STATE_OFFLINE;
    strncpy(s_state.vpn_ip,      "",        sizeof(s_state.vpn_ip)      - 1);
    strncpy(s_state.vpn_display, "OFFLINE", sizeof(s_state.vpn_display) - 1);

    /* ── Relays ─────────────────────────────────────────────────────────── */
    s_state.relay1_on = relay_get_state(1);
    s_state.relay2_on = relay_get_state(2);

    /* ── Core Temperature ───────────────────────────────────────────────── */
    if (s_temp_sensor != NULL) {
        float t = 0.0f;
        if (temperature_sensor_get_celsius(s_temp_sensor, &t) == ESP_OK) {
            s_state.core_temp_c = t;
        }
    } else {
        ESP_LOGD(TAG, "Temp sensor handle not set; reporting 0.0 C");
    }

    /* ── Heap / PSRAM ───────────────────────────────────────────────────── */
    s_state.free_heap_kb  =
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL) / 1024;
    s_state.free_psram_mb =
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / (1024 * 1024);

    /* ── Phone Battery ──────────────────────────────────────────────────── */
    s_state.phone_battery_pct = (int8_t)web_server_get_phone_battery();

    /* ── Live Log (last 10 lines from diag_log ring buffer) ─────────────── */
    diag_log_get_last_n_lines(s_state.log_lines, 10, 80);

    return &s_state;
}

const system_state_t *node_state_get(void) {
    return &s_state;
}
