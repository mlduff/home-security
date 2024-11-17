#include "esp_wifi.h"
#include <esp_err.h>
#include <esp_log.h>

#include "../storage/wifi_password.h"
#include "../storage/wifi_ssid.h"
#include "wifi.h"

static const char* TAG = "client:network";

esp_err_t network_start() {
    char* wifi_ssid = NULL;
    size_t wifi_ssid_length;
    esp_err_t ret = get_wifi_ssid(&wifi_ssid, &wifi_ssid_length);

    char* wifi_password = NULL;
    size_t wifi_password_length;
    ret = get_wifi_password(&wifi_password, &wifi_password_length);

    wifi_stop();

    ret = wifi_init_sta(
        wifi_ssid,
        wifi_ssid_length,
        wifi_password,
        wifi_password_length
    );
    
    if (ret != ESP_OK) {
        ESP_LOGI(TAG, "Couldn't connect to network or server, creating AP.");
        wifi_init_ap();
    }

    return ret;
}