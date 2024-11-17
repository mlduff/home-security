#include <stdint.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_check.h"

static const char* TAG = "client:nvs_helpers";

esp_err_t get_i32(const char* storage_key, int32_t *value) {
    nvs_handle_t handle;

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Reading ID...");
        err = nvs_get_i32(handle, storage_key, value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Successfully retrieved %s.", storage_key);
        } else {
            if (err == ESP_ERR_NVS_NOT_FOUND) {
                ESP_LOGE(TAG, "The value has not been initialized yet.");
            } else {
                ESP_LOGE(TAG, "Error reading ID: %s", esp_err_to_name(err));
            }
            *value = -1;
        }
    }

    nvs_close(handle);

    return err;
}

esp_err_t set_i32(const char* storage_key, int32_t value) {
    nvs_handle_t handle;

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    } else {
        err = nvs_set_i32(handle, storage_key, value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Successfully set %s.", storage_key);
            err = nvs_commit(handle);
        }
    }

    nvs_close(handle);

    return err;
}

esp_err_t get_string(const char* storage_key, char **value, size_t* length) {
    nvs_handle_t handle;

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Reading %s...", storage_key);
        nvs_get_str(handle, storage_key, NULL, length);
        *value = malloc(*length);
        err = nvs_get_str(handle, storage_key, *value, length);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Successfully retrieved %s.", storage_key);
        } else {
            if (err == ESP_ERR_NVS_NOT_FOUND) {
                ESP_LOGE(TAG, "The value has not been initialized yet");
            } else {
                ESP_LOGE(TAG, "Error reading %s: %s", storage_key, esp_err_to_name(err));
            }

            *length = 0;
        }
    }

    nvs_close(handle);

    return err;
}

esp_err_t set_string(char* storage_key, char *value) {
    nvs_handle_t handle;

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    } else {
        err = nvs_set_str(handle, storage_key, value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Successfully set %s.", storage_key);
            err = nvs_commit(handle);
        }
    }

    nvs_close(handle);

    return err;
}

esp_err_t get_bool(const char* storage_key, bool *value) {
    uint8_t temp_value;
    nvs_handle_t handle;

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Reading %s...", storage_key);
        err = nvs_get_u8(handle, storage_key, &temp_value);
        if (err == ESP_OK) {
            *value = temp_value == 0 ? false : true;
            ESP_LOGI(TAG, "Successfully retrieved %s.", storage_key);
        } else {
            if (err == ESP_ERR_NVS_NOT_FOUND) {
                ESP_LOGE(TAG, "The value has not been initialized yet.");
            } else {
                ESP_LOGE(TAG, "Error reading %s status: %s", storage_key, esp_err_to_name(err));
            }
        }
    }

    nvs_close(handle);

    return err;
}

esp_err_t set_bool(const char* storage_key, bool value) {
    nvs_handle_t handle;

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
    } else {
        uint8_t temp_value;
        if (value == true) {
            temp_value = 1;
        } else {
            temp_value = 0;
        }
        err = nvs_set_u8(handle, storage_key, temp_value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Successfully set %s status.", storage_key);
            err = nvs_commit(handle);
        }
    }

    nvs_close(handle);

    return err;
}

esp_err_t delete_value(const char* storage_key) {
    esp_err_t ret;

    nvs_handle_t handle;
    ESP_GOTO_ON_ERROR(nvs_open("storage", NVS_READWRITE, &handle), err, TAG, "Error opening handle.");

    ret = nvs_erase_key(handle, storage_key);

err:
    nvs_close(handle);
    return ret;
}
