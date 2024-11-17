#ifndef WIFI_SSID_H

#define WIFI_SSID_H

#include <esp_err.h>

esp_err_t get_wifi_ssid(char **value, size_t *length);
esp_err_t set_wifi_ssid(char *value);
esp_err_t delete_wifi_ssid();

#endif