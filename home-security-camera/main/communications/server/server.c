#include <esp_http_server.h>
#include <esp_log.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "esp_check.h"
#include <sys/param.h>
#include "esp_camera.h"

static const char *TAG = "server";

esp_err_t index_handler(httpd_req_t *req);
esp_err_t stream_handler(httpd_req_t *req);
esp_err_t network_configuration_handler(httpd_req_t *req);
esp_err_t network_configuration_options_handler(httpd_req_t *req);

/* URI handler structure for GET /uri */
httpd_uri_t get_index = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = index_handler,
    .user_ctx = NULL
};

/* URI handler structure for GET /uri */
httpd_uri_t get_stream = {
    .uri      = "/stream",
    .method   = HTTP_GET,
    .handler  = stream_handler,
    .user_ctx = NULL
};

httpd_uri_t post_network_configuration = {
    .uri      = "/network",
    .method   = HTTP_POST,
    .handler  = network_configuration_handler,
    .user_ctx = NULL
};

/* Function for starting the webserver */
httpd_handle_t server_start(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &get_index);
        httpd_register_uri_handler(server, &get_stream);
        httpd_register_uri_handler(server, &post_network_configuration);
    }
    if (server != NULL) {
        ESP_LOGI(TAG, "Server started successfully.");
    } else {
        ESP_LOGE(TAG, "Server did not start correctly.");
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void server_stop(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}