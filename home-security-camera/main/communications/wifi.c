/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "../storage/wifi_password.h"
#include "../storage/wifi_ssid.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi.h"
#include <esp_err.h>

// TODO: https://esp32.com/viewtopic.php?t=22486

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY
#define ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define ESP_WIFI_MAX_CONN CONFIG_ESP_WIFI_MAX_CONN

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "client:wifi";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;

static esp_netif_t *sta_netif = NULL;
static esp_netif_t *ap_netif = NULL;

static void copy_string_to_uint8(uint8_t *dest, const char *src, size_t dest_size)
{
    if (dest == NULL || src == NULL || dest_size == 0) {
        ESP_LOGE(TAG, "Invalid buffer or string");
        return;
    }
    strncpy((char *) dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d, reason=%d", MAC2STR(event->mac), event->aid, event->reason);
    }
}

void wifi_stop() {
    ESP_LOGI(TAG, "Stopping WiFi.");

    wifi_mode_t wifi_mode;
    ESP_ERROR_CHECK(esp_wifi_get_mode(&wifi_mode));
    if (wifi_mode == WIFI_MODE_STA) {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
    }
    ESP_ERROR_CHECK(esp_wifi_stop()); 
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
}

void wifi_init() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    s_wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler,
        NULL,
        &instance_any_id
    ));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        NULL,
        &instance_got_ip
    ));
}

esp_err_t wifi_init_sta(char *ssid, size_t ssid_length, char *password, size_t password_length)
{
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER
        }
    };

    if (ssid_length == 0 ||
        ssid_length >= sizeof(wifi_config.sta.ssid) ||
        password_length == 0 ||
        password_length >= sizeof(wifi_config.sta.password)
    ) {
        ESP_LOGI(TAG, "Invalid network credentials.");
        return ESP_ERR_INVALID_ARG;
    }

    copy_string_to_uint8(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    copy_string_to_uint8(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_LOGI(TAG, "Attempting to AP with SSID: %s and password: %s.", ssid, password);

    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    if (!sta_netif) {
        ESP_LOGI(TAG, "Creating sta_netif.");
        sta_netif = esp_netif_create_default_wifi_sta();
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(
            TAG,
            "Connected to AP SSID:%s, password:%s",
            ssid,
            password);
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(
            TAG,
            "Failed to connect to AP SSID:%s, password:%s",
            ssid,
            password);

        return ESP_FAIL;
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}

void wifi_init_ap(void)
{
    ESP_LOGI(TAG, "Initializing WiFi.");

    if (!ap_netif) {
        ESP_LOGI(TAG, "Creating ap_netif.");
        ap_netif = esp_netif_create_default_wifi_ap();
    }

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .password = ESP_WIFI_PASS,
            .max_connection = ESP_WIFI_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = false, // Disable PMF for compatibility
            },
        }};

    if (strlen(ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(
        TAG,
        "wifi_init_softap finished. SSID:%s password:%s channel:%d",
        ESP_WIFI_SSID,
        ESP_WIFI_PASS,
        ESP_WIFI_CHANNEL
    );
}