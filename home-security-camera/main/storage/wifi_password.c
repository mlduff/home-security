#include "nvs_helpers.h"

#define WIFI_PASSWORD_STORAGE_KEY "wifi_password"

esp_err_t get_wifi_password(char **value, size_t *length) {
    return get_string(WIFI_PASSWORD_STORAGE_KEY, value, length);
}

esp_err_t set_wifi_password(char *value) {
    return set_string(WIFI_PASSWORD_STORAGE_KEY, value);
}

esp_err_t delete_wifi_password() {
    return delete_value(WIFI_PASSWORD_STORAGE_KEY);
}