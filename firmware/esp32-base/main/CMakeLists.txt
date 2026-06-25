idf_component_register(
    SRCS "main.c" "ble_base.c" "ble_rover.c"
    INCLUDE_DIRS "."
    REQUIRES driver bt nvs_flash
)
