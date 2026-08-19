#include "relay.h"
#include "esp_log.h"

static const char *TAG = "RELAY";

// State tracking variables
static bool r1_state = false;
static bool r2_state = false;

void relay_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY_1_PIN) | (1ULL << RELAY_2_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // Default state: Both Relays OFF (Now driving LOW/0V)
    gpio_set_level(RELAY_1_PIN, 0); 
    gpio_set_level(RELAY_2_PIN, 0); 
    r1_state = false;
    r2_state = false;
    
    ESP_LOGI(TAG, "Relays initialized. R1(GPIO %d), R2(GPIO %d) set to OFF (LOW)", RELAY_1_PIN, RELAY_2_PIN);
}

void relay_set_state(int relay_num, bool on) {
    int pin = (relay_num == 1) ? RELAY_1_PIN : RELAY_2_PIN;
    
    // Flipped Logic: 1 = ON (HIGH), 0 = OFF (LOW)
    gpio_set_level(pin, on ? 1 : 0);
    
    if (relay_num == 1) r1_state = on;
    if (relay_num == 2) r2_state = on;

    ESP_LOGI(TAG, "Relay %d Command: %s (Driving Pin %d %s)", 
             relay_num, on ? "ON" : "OFF", pin, on ? "HIGH" : "LOW");
}

bool relay_get_state(int relay_num) {
    return (relay_num == 1) ? r1_state : r2_state;
}