idf_component_register(
    SRCS "bsp.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_driver_i2c esp_driver_gpio esp_lcd esp_lcd_touch_gt911 log
)
