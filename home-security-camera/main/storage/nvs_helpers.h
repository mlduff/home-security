#ifndef NVS_HELPERS
#define NVS_HELPERS

#include <esp_err.h>
#include "esp_system.h"

esp_err_t set_i32(const char* storage_key, int32_t value);
esp_err_t get_i32(const char* storage_key, int32_t *value);
esp_err_t get_string(const char* storage_key, char **value, size_t* length);
esp_err_t set_string(const char* storage_key, char *value);
esp_err_t get_bool(const char* storage_key, bool *value);
esp_err_t set_bool(const char* storage_key, bool value);
esp_err_t delete_value(const char* storage_key);

#endif