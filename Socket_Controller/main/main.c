/**
 * main.c
 *
 * Node-Aether Socket Controller — app_main entry point.
 *
 * Startup sequence:
 *   1. diag_log_init()           — capture crash snapshot before anything overwrites RTC RAM
 *   2. relay_init()              — GPIO setup (safe to do early)
 *   3. display_manager_init()    — SPI + ST7789 + LVGL + port task (Core 1)
 *   4. wifi_init()               — connect to configured AP
 *   5. Wait for WiFi             — block until IP assigned (5 s timeout)
 *   6. start_webserver()         — esp_httpd on port 8080
 *   7. display_controller_init() — register /screen/N and /slide/on|off on httpd
 *   8. node_state_set_temp_sensor() — inject temp handle from web_server
 *   9. dashboard_ui_init()       — build all 5 LVGL screens (runs in LVGL lock)
 *  10. lv_timer_create()         — 2 s refresh timer drives dashboard_ui_timer_cb
 *  11. diag_log_start_heap_monitor() — heap/log-relay tasks
 */

#include "display_manager.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "relay.h"
#include "diag_log.h"
#include "display_controller.h"
#include "node_state.h"
#include "dashboard_ui.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/temperature_sensor.h"
#include <stdio.h>

static const char *TAG = "MAIN";

/* Dashboard refresh — matches TICK_MS in dashboard_ui.c */
#define DASHBOARD_REFRESH_MS 2000

/* ── Helper: acquire a temp sensor handle for node_state ──────────────────── */
static temperature_sensor_handle_t init_temp_sensor(void) {
    temperature_sensor_handle_t handle = NULL;
    temperature_sensor_config_t cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 80);
    if (temperature_sensor_install(&cfg, &handle) == ESP_OK) {
        temperature_sensor_enable(handle);
        ESP_LOGI(TAG, "Temperature sensor ready");
    } else {
        ESP_LOGW(TAG, "Temperature sensor unavailable (web_server may own it)");
    }
    return handle;
}

void app_main(void) {
    /* ── 1. Diagnostic logging (must be first — captures crash snapshot) ─── */
    diag_log_init();
    ESP_LOGI(TAG, "=== Node-Aether Socket Controller booting ===");

    /* ── 2. Relay GPIO ───────────────────────────────────────────────────── */
    relay_init();

    /* ── 3. Display (SPI + ST7789 + LVGL port task on Core 1) ───────────── */
    ESP_ERROR_CHECK(display_manager_init());

    /* ── 4. WiFi ─────────────────────────────────────────────────────────── */
    wifi_init();

    /* ── 5. Wait up to 5 s for IP (non-blocking dashboard will show
     *       RECONNECTING until WiFi comes up on subsequent ticks) ────────── */
    EventBits_t bits = xEventGroupWaitBits(
        wifi_event_group, WIFI_CONNECTED_BIT,
        pdFALSE, pdTRUE, pdMS_TO_TICKS(5000)
    );
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi connected");
    } else {
        ESP_LOGW(TAG, "WiFi not connected within 5 s — continuing anyway");
    }

    /* ── 6. Web server (port 8080) ───────────────────────────────────────── */
    httpd_handle_t server = start_webserver();
    if (!server) {
        ESP_LOGE(TAG, "Failed to start web server");
    }

    /* ── 7. Display screen-switch HTTP endpoints (/screen/N, /slide/on|off) ── */
    display_controller_init(server);

    /* ── 8. Temperature sensor for node_state ────────────────────────────── */
    temperature_sensor_handle_t temp_handle = init_temp_sensor();
    node_state_set_temp_sensor(temp_handle);

    /* ── 9. Dashboard UI: build all 5 screens inside the LVGL lock ───────── */
    if (display_manager_lock(1000)) {
        /* Do an initial state snapshot so the UI has real data on first draw */
        node_state_tick();
        dashboard_ui_init();
        display_manager_unlock();
    } else {
        ESP_LOGE(TAG, "Could not acquire LVGL lock — dashboard not initialised");
    }

    /* ── 10. 2 s refresh timer — drives state updates + auto-slide ───────── */
    lv_timer_create(dashboard_ui_timer_cb, DASHBOARD_REFRESH_MS, NULL);

    /* ── 11. Heap monitor + TCP log-relay task ───────────────────────────── */
    diag_log_start_heap_monitor();

    ESP_LOGI(TAG, "Node-Aether boot complete. Dashboard running on Core 1.");

    /* app_main task can be deleted — all work is in LVGL port task + timers */
    vTaskDelete(NULL);
}