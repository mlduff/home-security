#include "nvs_helpers.h"

#define WIFI_SSID_STORAGE_KEY "wifi_ssid"

esp_err_t get_wifi_ssid(char **value, size_t *length) {
    return get_string(WIFI_SSID_STORAGE_KEY, value, length);
}

esp_err_t set_wifi_ssid(char *value) {
    return set_string(WIFI_SSID_STORAGE_KEY, value);
}

esp_err_t delete_wifi_ssid() {
    return delete_value(WIFI_SSID_STORAGE_KEY);
}