idf_component_register(
    SRCS "main.c" "waveshare_rgb_lcd_port.c" "lvgl_port.c" "batt_indicator.c"
         "ble_base_client.c" "wifi_provision.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c esp_driver_gpio esp_lcd esp_timer nvs_flash bt
)
