#ifndef _PANCHO_WEB_SERVER_H_
#define _PANCHO_WEB_SERVER_H_

#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h> 
#include "esp_err.h"
#include "esp_log.h"
#include <esp_http_server.h>

#define TAG CONFIG_TAG


httpd_handle_t config_web_server_init(httpd_uri_t** httpd_uri, uint16_t size) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "web server: Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        do {
            size--;
            ESP_LOGI(TAG, "web server: Registering %s handler", httpd_uri[size]->uri);
            httpd_register_uri_handler(server, httpd_uri[size]);
        } while (size!=0);       
        
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

#endif // _PANCHO_WEB_SERVER_H_
