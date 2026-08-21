/**
 * @file main.c
 * @brief MicroLink v2 Modular System (Tailscale + WOL + Web Dashboard)
 */

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

static const char *TAG = "main";

#define MSG_PORT        9000
#define MSG_SEND_INTERVAL_MS 5000

static microlink_t *ml = NULL;
static microlink_udp_socket_t *udp_sock = NULL;
static uint32_t msg_tx_count = 0;
static uint32_t msg_rx_count = 0;

/* ============================================================================
 * UDP RX Callback & Remote Command Router
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

    /* Command 1: Wake from Suspend/Hibernate via Ethernet WOL */
    if (strcasecmp(msg, "wake") == 0) {
        ESP_LOGI(TAG, "Received remote wake command! Firing WOL packet...");
        send_wake_on_lan();
        microlink_udp_send(sock, src_ip, src_port, "WOL packet sent!", 17);
        return;
    }

    /* Command 2: Hard Boot from Complete Shutdown via Wake-on-AC Relay Cycle */
    if (strcasecmp(msg, "powercycle") == 0 || strcasecmp(msg, "ac") == 0) {
        ESP_LOGI(TAG, "Received remote power cycle command! Cycling wall socket...");
        
        // Cut power for 3 seconds to drain motherboard capacitors
        relay_set_state(1, false); // OFF (Open / High)
        vTaskDelay(pdMS_TO_TICKS(3000));
        
        // Restore power -> triggers BIOS Wake-on-AC
        relay_set_state(1, true);  // ON (Closed / Low)
        
        microlink_udp_send(sock, src_ip, src_port, "AC power cycled! Wake on AC triggered.", 39);
        return;
    }

    /* Command 3: Generic Relay 1 On/Off */
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

    /* Command 4: Generic Relay 2 On/Off */
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

    /* Standard Echo Fallback */
    char reply[300];
    int reply_len = snprintf(reply, sizeof(reply), "ECHO: %s", msg);
    if (reply_len > 0) {
        microlink_udp_send(sock, src_ip, src_port, reply, reply_len);
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

    if (state == ML_STATE_CONNECTED) {
        uint32_t ip = microlink_get_vpn_ip(ml_handle);
        char ip_str[16];
        microlink_ip_to_str(ip, ip_str);
        ESP_LOGI(TAG, "Connected to Tailscale! VPN IP: %s", ip_str);
        ESP_LOGI(TAG, "Open Dashboard in browser at: http://%s:8080/", ip_str);
    }
}

static void on_peer_update(microlink_t *ml_handle, const microlink_peer_info_t *peer, void *user_data) {
    char ip_str[16];
    microlink_ip_to_str(peer->vpn_ip, ip_str);
    ESP_LOGI(TAG, "Peer: %s (%s) online=%d direct=%d", peer->hostname, ip_str, peer->online, peer->direct_path);
}

/* ============================================================================
 * Main Application Orchestration
 * ========================================================================== */

void app_main(void) {
    /* Initialize NVS */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Starting Modular Power Controller System...");

    /* Initialize WiFi via manager module */
    wifi_init();

    /* Wait for WiFi connection */
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    /* Initialize Relay Module (Starts OFF / Open) */
    relay_init();

    /* Initialize Ethernet W5500 gracefully & start WOL Button Task */
    gpio_install_isr_service(0);
    init_w5500_ethernet();
    xTaskCreate(wol_button_task, "wol_btn_task", 4096, NULL, 5, NULL);

    /* Initialize MicroLink Tailscale Connection */
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
    microlink_set_peer_callback(ml, on_peer_update, NULL);
    ESP_ERROR_CHECK(microlink_start(ml));

    /* Wait for CONNECTED state before proceeding */
    while (!microlink_is_connected(ml)) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /* Start UDP communication socket */
    udp_sock = microlink_udp_create(ml, MSG_PORT);
    if (!udp_sock) {
        ESP_LOGE(TAG, "Failed to create UDP socket");
    } else {
        ESP_LOGI(TAG, "UDP socket listening on port %d", MSG_PORT);
        microlink_udp_set_rx_callback(udp_sock, on_udp_rx, NULL);
    }

    /* Start the HTTP Web Server & Dashboard */
    start_webserver();

    uint32_t target_ip = 0;
    const char *target_ip_str = CONFIG_ML_EXAMPLE_TARGET_PEER_IP;
    if (target_ip_str && target_ip_str[0] != '\0') {
        target_ip = microlink_parse_ip(target_ip_str);
    }

    uint64_t last_send_ms = 0;
    uint64_t last_status_ms = 0;

    /* Main heartbeat and status loop */
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        uint64_t now = (uint64_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

        // Optional periodic messaging loop
        if (udp_sock && target_ip != 0 && now - last_send_ms >= MSG_SEND_INTERVAL_MS) {
            last_send_ms = now;
            msg_tx_count++;
            char msg[128];
            int msg_len = snprintf(msg, sizeof(msg), "heartbeat #%lu", (unsigned long)msg_tx_count);
            microlink_udp_send(udp_sock, target_ip, MSG_PORT, msg, msg_len);
        }

        // Periodic system status logging every 30s
        if (now - last_status_ms >= 30000) {
            last_status_ms = now;
            if (microlink_is_connected(ml)) {
                ESP_LOGI(TAG, "System Status: CONNECTED | Peers: %d | Free Heap: %lu bytes",
                         microlink_get_peer_count(ml), (unsigned long)esp_get_free_heap_size());
            }
        }
    }
}