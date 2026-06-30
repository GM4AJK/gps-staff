idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES bsp theme screens esp_lcd esp_timer freertos log
)
