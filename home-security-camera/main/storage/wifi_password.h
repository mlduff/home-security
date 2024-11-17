#ifndef WIFI_PASSWORD_H

#define WIFI_PASSWORD_H

#include <esp_err.h>

esp_err_t get_wifi_password(char **value, size_t *length);
esp_err_t set_wifi_password(char *value);
esp_err_t delete_wifi_password();

#endif