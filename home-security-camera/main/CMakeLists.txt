set(srcs 
    "main.c"
    "camera/camera.c"
    "communications/network.c"
    "communications/wifi.c"
    "communications/server/server.c"
    "communications/server/handlers/index.c"
    "communications/server/handlers/configuration.c"
    "communications/server/handlers/stream.c"
    "storage/spiffs.c"
    "storage/nvs_helpers.c"
    "storage/wifi_password.c"
    "storage/wifi_ssid.c"
)

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "."
)

spiffs_create_partition_image(spiffs spiffs FLASH_IN_PROJECT)