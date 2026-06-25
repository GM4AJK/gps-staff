idf_component_register(
    SRCS "main.c" "waveshare_rgb_lcd_port.c" "lvgl_port.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_i2c esp_driver_gpio esp_lcd esp_timer
)
