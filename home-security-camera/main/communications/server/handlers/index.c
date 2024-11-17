#include <esp_err.h>
#include <esp_http_server.h>
#include "esp_log.h"

static const char* TAG = "client:index";

esp_err_t index_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received index request, serving webpage.");
    // Open the HTML file from SPIFFS
    FILE* f = fopen("/spiffs/index.html", "r");
    if (f == NULL) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    // Read the file content and send as response
    char buf[1024];
    size_t read_bytes;
    while ((read_bytes = fread(buf, 1, sizeof(buf), f)) > 0) {
        httpd_resp_send_chunk(req, buf, read_bytes);
    }
    fclose(f);

    // Send empty chunk to signal the end of response
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}