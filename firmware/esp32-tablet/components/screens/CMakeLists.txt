idf_component_register(
    SRCS
        "screen_home.c"
        "screen_base_config.c"
    INCLUDE_DIRS "."
    REQUIRES lvgl__lvgl theme log
)
