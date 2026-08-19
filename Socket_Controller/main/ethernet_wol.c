#include "ethernet_wol.h"
#include <string.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

esp_eth_handle_t eth_handle = NULL;
static const char *TAG = "WOL_ETH";

#define SPI_MISO_PIN  13
#define SPI_MOSI_PIN  11
#define SPI_SCLK_PIN  12
#define SPI_CS_PIN    10
#define W5500_INT_PIN 14
#define W5500_RST_PIN 9

void init_w5500_ethernet(void) {
    ESP_LOGI(TAG, "Initializing SPI Bus...");
    spi_bus_config_t buscfg = {
        .miso_io_num = SPI_MISO_PIN,
        .mosi_io_num = SPI_MOSI_PIN,
        .sclk_io_num = SPI_SCLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    
    // Initialize SPI bus, but ignore "invalid state" if it's already initialized
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus. Continuing without Ethernet.");
        return;
    }

    spi_device_interface_config_t devcfg = {
        .command_bits = 16,
        .address_bits = 8,
        .mode = 0,
        .clock_speed_hz = 20 * 1000 * 1000,
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 20
    };

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;
    phy_config.reset_gpio_num = W5500_RST_PIN;

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(SPI2_HOST, &devcfg);
    w5500_config.int_gpio_num = W5500_INT_PIN;

    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);
    
    ESP_LOGI(TAG, "Attempting to detect and install W5500 Ethernet module...");
    
    // GRACEFUL CHECK: Do not use ESP_ERROR_CHECK here!
    err = esp_eth_driver_install(&eth_config, &eth_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "W5500 module timed out or not detected!");
        ESP_LOGW(TAG, "Continuing without Ethernet WOL capability. Plug it in and reboot later to enable.");
        
        // Clean up MAC and PHY objects to prevent memory leaks
        if (mac) mac->del(mac);
        if (phy) phy->del(phy);
        
        return; // Safely exit the function without crashing
    }

    // GRACEFUL CHECK: Start the Ethernet driver
    err = esp_eth_start(eth_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Ethernet driver! Continuing without it.");
        return;
    }
    
    ESP_LOGI(TAG, "W5500 Initialized and Link Started Successfully!");
}

void send_wake_on_lan(void) {
    uint8_t target_mac[6] = {0x60, 0x18, 0x95, 0x12, 0xB5, 0x83};
    uint8_t frame[116];

    memset(&frame[0], 0xFF, 6);
    uint8_t src_mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
    memcpy(&frame[6], src_mac, 6);

    frame[12] = 0x08;
    frame[13] = 0x42;

    memset(&frame[14], 0xFF, 6);
    for (int i = 0; i < 16; i++) {
        memcpy(&frame[20 + (i * 6)], target_mac, 6);
    }

    ESP_LOGI(TAG, "Transmitting Raw Layer 2 Magic Packet via W5500...");
    if (eth_handle == NULL) {
        ESP_LOGW(TAG, "Ethernet handle is NULL! Cannot transmit WOL packet because module is disconnected.");
        return;
    }

    esp_err_t err = esp_eth_transmit(eth_handle, frame, sizeof(frame));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit raw frame, error: %d", err);
    } else {
        ESP_LOGI(TAG, "Raw Magic Packet transmitted successfully!");
    }
}

void wol_button_task(void *arg) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << 0),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    while (1) {
        if (gpio_get_level(GPIO_NUM_0) == 0) {
            ESP_LOGI(TAG, "BOOT button pressed! Firing Magic Packet...");
            send_wake_on_lan();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}