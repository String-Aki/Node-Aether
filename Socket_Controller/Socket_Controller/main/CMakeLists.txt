idf_component_register(
    SRCS "web_server.c" "main.c" "relay.c" "wifi_manager.c" "ethernet_wol.c" "web_server.c"
    INCLUDE_DIRS "."
    REQUIRES microlink nvs_flash esp_wifi esp_netif esp_event esp_eth
)
