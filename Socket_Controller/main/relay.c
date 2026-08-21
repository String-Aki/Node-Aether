#include "relay.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "RELAY";

// State tracking variables
static bool r1_state = false;
static bool r2_state = false;

#define NVS_NAMESPACE "relay_state"
#define R1_KEY "r1"
#define R2_KEY "r2"

void relay_init(void) {
    // 1. Read the saved state from NVS flash memory
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
    
    if (err == ESP_OK) {
        uint8_t val1 = 0, val2 = 0;
        if (nvs_get_u8(my_handle, R1_KEY, &val1) == ESP_OK) r1_state = val1;
        if (nvs_get_u8(my_handle, R2_KEY, &val2) == ESP_OK) r2_state = val2;
        nvs_close(my_handle);
        ESP_LOGI(TAG, "Restored state from NVS - R1: %d, R2: %d", r1_state, r2_state);
    } else {
        ESP_LOGW(TAG, "No previous state found in NVS, defaulting to OFF.");
    }

    // 2. Configure the hardware pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY_1_PIN) | (1ULL << RELAY_2_PIN),
        .mode = GPIO_MODE_OUTPUT, // Pure OUTPUT for maximum drive strength
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // 3. Explicitly force the hardware to match our restored NVS state
    // Flipped Logic: 1 = ON (HIGH), 0 = OFF (LOW)
    gpio_set_level(RELAY_1_PIN, r1_state ? 1 : 0);
    gpio_set_level(RELAY_2_PIN, r2_state ? 1 : 0);
    
    ESP_LOGI(TAG, "Relays initialized. R1: %d, R2: %d", r1_state, r2_state);
}

void relay_set_state(int relay_num, bool on) {
    int pin = (relay_num == 1) ? RELAY_1_PIN : RELAY_2_PIN;
    
    // Execute physical hardware change
    gpio_set_level(pin, on ? 1 : 0);
    
    // Update local variables
    if (relay_num == 1) r1_state = on;
    if (relay_num == 2) r2_state = on;
    
    ESP_LOGI(TAG, "Relay %d Command: %s (Driving Pin %d %s)", 
             relay_num, on ? "ON" : "OFF", pin, on ? "HIGH" : "LOW");

    // Save the new state instantly to NVS
    nvs_handle_t my_handle;
    if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle) == ESP_OK) {
        nvs_set_u8(my_handle, (relay_num == 1) ? R1_KEY : R2_KEY, on ? 1 : 0);
        nvs_commit(my_handle);
        nvs_close(my_handle);
    } else {
        ESP_LOGE(TAG, "Failed to open NVS to save Relay %d state", relay_num);
    }
}

bool relay_get_state(int relay_num) {
    return (relay_num == 1) ? r1_state : r2_state;
}