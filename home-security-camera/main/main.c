#include <esp_http_server.h>
#include "esp_camera.h"
#include <esp_log.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include "camera/camera.h"
#include "communications/wifi.h"
#include "communications/network.h"
#include "communications/server/server.h"
#include "storage/spiffs.h"

static const char *TAG = "main";

void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(init_spiffs());
    ESP_ERROR_CHECK(camera_init());
    wifi_init();
    ESP_ERROR_CHECK(network_start());
    httpd_handle_t server = server_start();
    if (server == NULL) {
        ESP_LOGE(TAG, "Server did not start correctly.");
    }
}
