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

/**
 * \brief decodes a string from the http standar i utf8 to utf8
 * \return new string
 * \warning the string length could be changed
 */
char* decode_web_param(char * param) {
    size_t max = strlen(param);
    uint16_t symbol_code;
    size_t i;
    size_t j;


    for (i = 0; i < max;i++) {
        if (param[i] == '\0')
            break;

        if (param[i] == '+') {
            param[i] = ' ';
        }

        if (param[i] == '%') {
            // loads the lower 8 bites with the fist one and the upper 8 bites with the second character
            symbol_code = ( 0xFF00 & ((uint16_t)param[i+1] << 8)) | (0x00FF & (uint16_t)param[i+2]);
            switch (symbol_code) {
                case 0X3231: param[i] = '!';  break;
                case 0X3232: param[i] = '"';  break;
                case 0X3233: param[i] = '#';  break;
                case 0X3234: param[i] = '$';  break;
                case 0X3235: param[i] = '%';  break;
                case 0X3236: param[i] = '&';  break;
                case 0X3237: param[i] = '\''; break;
                case 0X3238: param[i] = '(';  break;
                case 0X3239: param[i] = ')';  break;
                case 0X3242: param[i] = '+';  break;
                case 0X3243: param[i] = ',';  break;
                case 0X3246: param[i] = '/';  break;
                case 0X3341: param[i] = ':';  break;
                case 0X3342: param[i] = ';';  break;
                case 0X3343: param[i] = '<';  break;
                case 0X3345: param[i] = '>';  break;
                case 0X3344: param[i] = '=';  break;
                case 0X3346: param[i] = '?';  break;
                case 0X3430: param[i] = '@';  break;
                case 0X3542: param[i] = '[';  break;
                case 0X3543: param[i] = '\\'; break;
                case 0X3544: param[i] = ']';  break;
                case 0X3545: param[i] = '^';  break;
                case 0X3743: param[i] = '|';  break;
                case 0X3745: param[i] = '~';  break;
                default: ESP_LOGE(TAG, "unhandled symbol: %#06X", symbol_code); break;
            }
            ESP_LOGI(TAG, "interpreting symbol: %#06X as %c", symbol_code, param[i]);
            max-=2;
            for (j = i+1; j < max; j++)
                param[j] = param[j+2];
            param[max]='\0';
        }
    }
    return param;
}

#endif // _PANCHO_WEB_SERVER_H_
