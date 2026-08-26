#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"

#include "microlink.h"
#include "microlink_internal.h"

#include "wifi_manager.h"
#include "ethernet_wol.h"
#include "relay.h"
#include "web_server.h"
#include "diag_log.h"
#include "led_strip.h"
#include "display_manager.h"

#define ENABLE_ONBOARD_LED 0  /* Set to 1 to re-enable the built-in LED */

static const char *TAG = "main";

#if ENABLE_ONBOARD_LED
static led_strip_handle_t led_strip_handle = NULL;

typedef enum {
    LED_PATTERN_CONNECTING,    // Slow blinking yellow
    LED_PATTERN_REGISTERING,   // Rapid blinking blue
    LED_PATTERN_CONNECTED,     // Smooth breathing emerald pulse
    LED_PATTERN_ERROR,         // Rapid blinking red
    LED_PATTERN_OFF
} led_pattern_t;

static volatile led_pattern_t s_led_pattern = LED_PATTERN_CONNECTING;

static void led_animation_task(void *arg) {
    uint8_t brightness = 2;
    int8_t fade_direction = 1;

    while (1) {
        if (!led_strip_handle) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        switch (s_led_pattern) {
            case LED_PATTERN_CONNECTED:
                // Smooth Breathing Pulse (cycles brightness smoothly between 2 and 35)
                brightness += fade_direction;
                if (brightness >= 35) {
                    fade_direction = -1;
                } else if (brightness <= 2) {
                    fade_direction = 1;
                }

                // Emerald Tint: Green with a touch of Blue for a clean modern glow
                led_strip_set_pixel(led_strip_handle, 0, 0, brightness, brightness / 6);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(40)); // ~2.6 second complete breath cycle
                break;

            case LED_PATTERN_CONNECTING:
                // Blinking Yellow
                led_strip_set_pixel(led_strip_handle, 0, 30, 20, 0);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(500));
                led_strip_clear(led_strip_handle);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(500));
                break;

            case LED_PATTERN_REGISTERING:
                // Rapid Blinking Blue
                led_strip_set_pixel(led_strip_handle, 0, 0, 0, 35);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(200));
                led_strip_clear(led_strip_handle);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(200));
                break;

            case LED_PATTERN_ERROR:
                // Fast Warning Red Flash
                led_strip_set_pixel(led_strip_handle, 0, 40, 0, 0);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(150));
                led_strip_clear(led_strip_handle);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(150));
                break;

            default:
                led_strip_clear(led_strip_handle);
                led_strip_refresh(led_strip_handle);
                vTaskDelay(pdMS_TO_TICKS(200));
                break;
        }
    }
}
#endif

#define MSG_PORT        9000
#define MSG_SEND_INTERVAL_MS 5000

static microlink_t *ml = NULL;
static microlink_udp_socket_t *udp_sock = NULL;
static uint32_t msg_tx_count = 0;
static uint32_t msg_rx_count = 0;

/* ============================================================================
 * UDP RX Callback & Remote Command Router (For WOL / Wake on AC)
 * ========================================================================== */

static void on_udp_rx(microlink_udp_socket_t *sock, uint32_t src_ip, uint16_t src_port,
                       const uint8_t *data, size_t len, void *user_data) {
    msg_rx_count++;
    char ip_str[16];
    microlink_ip_to_str(src_ip, ip_str);

    char msg[256];
    size_t copy_len = (len < sizeof(msg) - 1) ? len : sizeof(msg) - 1;
    memcpy(msg, data, copy_len);
    msg[copy_len] = '\0';
    if (copy_len > 0 && msg[copy_len - 1] == '\n') msg[copy_len - 1] = '\0';

    ESP_LOGI(TAG, "UDP RX from %s:%u: \"%s\"", ip_str, src_port, msg);

    if (strcasecmp(msg, "wake") == 0) {
        ESP_LOGI(TAG, "Received remote wake command! Firing WOL packet...");
        send_wake_on_lan();
        microlink_udp_send(sock, src_ip, src_port, "WOL packet sent!", 17);
        return;
    }

    if (strcasecmp(msg, "powercycle") == 0 || strcasecmp(msg, "ac") == 0) {
        ESP_LOGI(TAG, "Received remote power cycle command! Cycling wall socket...");
        relay_set_state(1, false);
        vTaskDelay(pdMS_TO_TICKS(3000));
        relay_set_state(1, true);
        microlink_udp_send(sock, src_ip, src_port, "AC power cycled! Wake on AC triggered.", 39);
        return;
    }

    if (strcasecmp(msg, "relay1_on") == 0) {
        relay_set_state(1, true);
        microlink_udp_send(sock, src_ip, src_port, "Relay 1 ON", 10);
        return;
    }
    if (strcasecmp(msg, "relay1_off") == 0) {
        relay_set_state(1, false);
        microlink_udp_send(sock, src_ip, src_port, "Relay 1 OFF", 11);
        return;
    }

    if (strcasecmp(msg, "relay2_on") == 0) {
        relay_set_state(2, true);
        microlink_udp_send(sock, src_ip, src_port, "Relay 2 ON", 10);
        return;
    }
    if (strcasecmp(msg, "relay2_off") == 0) {
        relay_set_state(2, false);
        microlink_udp_send(sock, src_ip, src_port, "Relay 2 OFF", 11);
        return;
    }
}

/* ============================================================================
 * MicroLink Callbacks
 * ========================================================================== */

static void on_state_change(microlink_t *ml_handle, microlink_state_t state, void *user_data) {
    const char *state_names[] = {
        "IDLE", "WIFI_WAIT", "CONNECTING", "REGISTERING",
        "CONNECTED", "RECONNECTING", "ERROR"
    };
    const char *name = (state < sizeof(state_names)/sizeof(state_names[0]))
                       ? state_names[state] : "UNKNOWN";
    ESP_LOGI(TAG, "MicroLink state: %s", name);

#if ENABLE_ONBOARD_LED
    // Update active animation pattern
    switch(state) {
        case ML_STATE_WIFI_WAIT:
        case ML_STATE_CONNECTING:
            s_led_pattern = LED_PATTERN_CONNECTING;
            break;
        case ML_STATE_REGISTERING:
            s_led_pattern = LED_PATTERN_REGISTERING;
            break;
        case ML_STATE_CONNECTED:
            s_led_pattern = LED_PATTERN_CONNECTED;
            break;
        case ML_STATE_RECONNECTING:
        case ML_STATE_ERROR:
            s_led_pattern = LED_PATTERN_ERROR;
            break;
        default:
            s_led_pattern = LED_PATTERN_OFF;
            break;
    }
#endif

    if (state == ML_STATE_CONNECTED) {
        uint32_t ip = microlink_get_vpn_ip(ml_handle);
        char ip_str[16];
        microlink_ip_to_str(ip, ip_str);
        ESP_LOGI(TAG, "Connected to Tailscale! VPN IP: %s", ip_str);
        ESP_LOGI(TAG, "Open Dashboard in browser at: http://%s:8080/", ip_str);
    }
}

/* ============================================================================
 * Main Application Orchestration
 * ========================================================================== */

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize diagnostic logger IMMEDIATELY after NVS to catch early boot crashes
    diag_log_init();

    /* ── Display Init & Boot Splash ─────────────────────────────────────── */
    ESP_LOGI(TAG, "Initializing Display Manager...");
    ESP_ERROR_CHECK(display_manager_init());

    if (display_manager_lock(1000)) {
        lv_obj_t *scr = lv_scr_act();

        /* Background: near-black navy */
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x0D1117), 0);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

        /* ── Header bar ──────────────────────────────────────────────── */
        lv_obj_t *header = lv_obj_create(scr);
        lv_obj_set_size(header, 320, 42);
        lv_obj_set_pos(header, 0, 0);
        lv_obj_set_style_bg_color(header, lv_color_hex(0x0AA8D4), 0); /* Cyan-blue */
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_style_radius(header, 0, 0);
        lv_obj_set_style_pad_all(header, 0, 0);

        lv_obj_t *title = lv_label_create(header);
        lv_label_set_text(title, "  NODE-AETHER  |  Socket Controller");
        lv_obj_set_style_text_color(title, lv_color_hex(0x0D1117), 0);
        lv_obj_align(title, LV_ALIGN_LEFT_MID, 8, 0);

        /* ── Centre status line ──────────────────────────────────────── */
        lv_obj_t *status_lbl = lv_label_create(scr);
        lv_label_set_text(status_lbl, LV_SYMBOL_OK "  Display OK  —  System Booting...");
        lv_obj_set_style_text_color(status_lbl, lv_color_hex(0x00FF99), 0); /* Mint green */
        lv_obj_align(status_lbl, LV_ALIGN_CENTER, 0, -18);

        /* ── Subtitle / build info ───────────────────────────────────── */
        lv_obj_t *sub_lbl = lv_label_create(scr);
        lv_label_set_text(sub_lbl, "ESP32-S3  |  ESP-IDF v5.x  |  LVGL 8.3");
        lv_obj_set_style_text_color(sub_lbl, lv_color_hex(0x4A6080), 0); /* Muted slate */
        lv_obj_align(sub_lbl, LV_ALIGN_CENTER, 0, 6);

        /* ── Bottom status badges ────────────────────────────────────── */
        /* WiFi — amber (waiting to connect) */
        lv_obj_t *wifi_box = lv_obj_create(scr);
        lv_obj_set_size(wifi_box, 90, 34);
        lv_obj_align(wifi_box, LV_ALIGN_BOTTOM_LEFT, 8, -8);
        lv_obj_set_style_bg_color(wifi_box, lv_color_hex(0xF59E0B), 0);
        lv_obj_set_style_border_width(wifi_box, 0, 0);
        lv_obj_set_style_radius(wifi_box, 6, 0);
        lv_obj_set_style_pad_all(wifi_box, 0, 0);
        lv_obj_t *wifi_lbl = lv_label_create(wifi_box);
        lv_label_set_text(wifi_lbl, LV_SYMBOL_WIFI "  WiFi");
        lv_obj_set_style_text_color(wifi_lbl, lv_color_hex(0x0D1117), 0);
        lv_obj_align(wifi_lbl, LV_ALIGN_CENTER, 0, 0);

        /* VPN — grey (offline) */
        lv_obj_t *vpn_box = lv_obj_create(scr);
        lv_obj_set_size(vpn_box, 90, 34);
        lv_obj_align(vpn_box, LV_ALIGN_BOTTOM_MID, 0, -8);
        lv_obj_set_style_bg_color(vpn_box, lv_color_hex(0x1E2D3D), 0);
        lv_obj_set_style_border_color(vpn_box, lv_color_hex(0x2D4560), 0);
        lv_obj_set_style_border_width(vpn_box, 1, 0);
        lv_obj_set_style_radius(vpn_box, 6, 0);
        lv_obj_set_style_pad_all(vpn_box, 0, 0);
        lv_obj_t *vpn_lbl = lv_label_create(vpn_box);
        lv_label_set_text(vpn_lbl, LV_SYMBOL_EYE_CLOSE "  VPN");
        lv_obj_set_style_text_color(vpn_lbl, lv_color_hex(0x4A6080), 0);
        lv_obj_align(vpn_lbl, LV_ALIGN_CENTER, 0, 0);

        /* Relay — grey (uninitialised) */
        lv_obj_t *relay_box = lv_obj_create(scr);
        lv_obj_set_size(relay_box, 90, 34);
        lv_obj_align(relay_box, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
        lv_obj_set_style_bg_color(relay_box, lv_color_hex(0x1E2D3D), 0);
        lv_obj_set_style_border_color(relay_box, lv_color_hex(0x2D4560), 0);
        lv_obj_set_style_border_width(relay_box, 1, 0);
        lv_obj_set_style_radius(relay_box, 6, 0);
        lv_obj_set_style_pad_all(relay_box, 0, 0);
        lv_obj_t *relay_lbl = lv_label_create(relay_box);
        lv_label_set_text(relay_lbl, LV_SYMBOL_POWER "  Relay");
        lv_obj_set_style_text_color(relay_lbl, lv_color_hex(0x4A6080), 0);
        lv_obj_align(relay_lbl, LV_ALIGN_CENTER, 0, 0);

        display_manager_unlock();
    }
    /* ─────────────────────────────────────────────────────────────────── */

#if ENABLE_ONBOARD_LED
    // Initialize the RGB LED on GPIO 48
    led_strip_config_t strip_config = {
        .strip_gpio_num = 48,
        .max_leds = 1, // Only one LED on the board
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
    };
    if (led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip_handle) == ESP_OK) {
        led_strip_clear(led_strip_handle);
        // Start background animation task
        xTaskCreate(led_animation_task, "led_anim", 2048, NULL, 2, NULL);
    }
#endif

    ESP_LOGI(TAG, "Starting Modular Power Controller System...");

    wifi_init();
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    relay_init();

    // Start the periodic heap monitor now that core systems are up
    diag_log_start_heap_monitor();

    gpio_install_isr_service(0);
    init_w5500_ethernet();
    xTaskCreate(wol_button_task, "wol_btn_task", 4096, NULL, 5, NULL);

    microlink_config_t config = {
        .auth_key = CONFIG_ML_TAILSCALE_AUTH_KEY,
        .device_name = CONFIG_ML_DEVICE_NAME,
        .enable_derp = true,
        .enable_stun = true,
        .enable_disco = true,
        .max_peers = CONFIG_ML_MAX_PEERS,
        .wifi_tx_power_dbm = 13,
    };

    ml = microlink_init(&config);
    if (!ml) {
        ESP_LOGE(TAG, "Failed to initialize MicroLink");
        return;
    }

    microlink_set_state_callback(ml, on_state_change, NULL);
    ESP_ERROR_CHECK(microlink_start(ml));

    while (!microlink_is_connected(ml)) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    udp_sock = microlink_udp_create(ml, MSG_PORT);
    if (udp_sock) {
        ESP_LOGI(TAG, "UDP socket listening on port %d", MSG_PORT);
        microlink_udp_set_rx_callback(udp_sock, on_udp_rx, NULL);
    }

    start_webserver();

    uint64_t last_status_ms = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        uint64_t now = (uint64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

        // --- SCHEDULED WEEKLY RESTART ---
        // 7 days = 7 * 24 * 60 * 60 * 1000 = 604800000 milliseconds
        if (now >= 604800000ULL) {
            ESP_LOGW(TAG, "7-day uptime reached. Performing scheduled weekly restart.");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Give the log a second to print/stream
            esp_restart();
        }

        if (now - last_status_ms >= 30000) {
            last_status_ms = now;
            if (microlink_is_connected(ml)) {
                ESP_LOGI(TAG, "System Status: CONNECTED | Free Heap: %lu bytes",
                         (unsigned long)esp_get_free_heap_size());
            }
        }
    }
}