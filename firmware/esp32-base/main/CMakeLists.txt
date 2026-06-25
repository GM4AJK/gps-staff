idf_component_register(
    SRCS "main.c" "ble_base.c" "ble_rover.c"
    INCLUDE_DIRS "."
    REQUIRES bt nvs_flash esp_driver_uart esp_driver_gpio
)
