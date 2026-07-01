idf_component_register(
    SRCS
        "screen_home.c"
        "screen_base_config.c"
        "screen_wifi_networks.c"
    INCLUDE_DIRS "."
    REQUIRES lvgl__lvgl theme log
)
