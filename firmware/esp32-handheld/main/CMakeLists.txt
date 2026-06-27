idf_component_register(
    SRCS "main.c" "waveshare_rgb_lcd_port.c" "lvgl_port.c" "batt_indicator.c"
         "home_screen.c" "base_config.c" "wifi_provision.c" "logo_img.c"
         "ble_base_client.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c esp_driver_gpio esp_lcd esp_timer nvs_flash bt
)
