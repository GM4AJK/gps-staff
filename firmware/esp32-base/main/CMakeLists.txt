idf_component_register(
    SRCS "main.c" "ble_base.c" "wifi_prov.c"
    INCLUDE_DIRS "."
    REQUIRES bt nvs_flash esp_driver_gpio esp_wifi esp_netif esp_event
)
