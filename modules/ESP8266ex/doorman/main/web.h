#ifndef _PANCHO_WEB_H_
#define _PANCHO_WEB_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "../../include/web_server.h"
#include "mqtt_handler.h"

#define TAG CONFIG_TAG


void handle_config_parameter(char* param, char* value) {
    param = decode_web_param(param);
    value = decode_web_param(value);

    if (strcmp(param, "STA_WIFI_SSID") == 0)
        strcpy(STA_WIFI_SSID, value);
    
    if (strcmp(param, "STA_WIFI_PASSWORD") == 0)
        strcpy(STA_WIFI_PASSWORD, value);

    if (strcmp(param, "BROKER_URL") == 0)
        strcpy(BROKER_URL, value);

    if (strcmp(param, "BROKER_PORT") == 0)
        BROKER_PORT = atoi(value);

    if (strcmp(param, "BROKER_USER") == 0)
        strcpy(BROKER_USER, value);
 
    if (strcmp(param, "BROKER_PASSWORD") == 0)
        strcpy(BROKER_PASSWORD, value);

    if (strcmp(param, "MQTT_TOPIC") == 0)
        strcpy(MQTT_TOPIC, value);

    if (strcmp(param, "MQTT_LIGHTS_TOPICS") == 0)
        strcpy(MQTT_LIGHTS_TOPICS, value);

    if (strcmp(param, "LIGHTS_INTENSITY_OPEN") == 0)
        strcpy(LIGHTS_INTENSITY_OPEN, value);

    if (strcmp(param, "LIGHTS_INTENSITY_CLOSE") == 0)
        strcpy(LIGHTS_INTENSITY_CLOSE, value);

    ESP_LOGI(TAG, "param: %s:%s", param, value);
}


/* An HTTP POST handler */
esp_err_t configuration_post_handler(httpd_req_t *req) {
    char buf[1024];
    int ret, remaining;
    char* pch;
    char param[25];
    char value[75];
    char response[100];

    bzero(buf, 1024);
    remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                        MIN(remaining, sizeof(buf)))) <= 0) {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        remaining -= ret;

        /* Log data received */
        ESP_LOGI(TAG, "=========== RECEIVED CONFIG DATA ===========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "============================================");
    }

    pch = strtok(buf, "=");
    strcpy(param, pch);
    pch = strtok(NULL, "&");
    strcpy(value, pch);
    while (param != NULL && value != NULL) {
        handle_config_parameter(param, value);
        pch = strtok(NULL, "=");
        if (pch == NULL)
            break;
        strcpy(param, pch);
        pch = strtok(NULL, "&");
        strcpy(value, pch);
    }
    strcpy(response, "The configuration was saved, please reload the page to change them again.");

    httpd_resp_send(req, response, strlen(response));
    // httpd_resp_send_chunk(req, response, strlen(response));
    save_node_configuration();
    return ESP_OK;
}

esp_err_t configuration_handler(httpd_req_t *req) {
    char broker_port_char[8];
    char* resp_str = (char*) malloc(255 * sizeof(char));
    char resp_str_aux [1024];
    bzero(resp_str, 255);
    bzero(resp_str_aux, 1024);

    ESP_LOGI(TAG, "web: building web page");

    itoa(BROKER_PORT, broker_port_char, 10);

    strcpy(resp_str_aux, (char * restrict)"<meta charset='utf-8'><style>label {display: inline-block; width: 150px; text-align: right;}</style><form method='post'><div><label for='STA_WIFI_SSID'>wifi name:</label><input type='text' name='STA_WIFI_SSID' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, STA_WIFI_SSID);

    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='STA_WIFI_PASSWORD'>wifi password:</label><input type='text' name='STA_WIFI_PASSWORD' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, STA_WIFI_PASSWORD);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_URL'>mqtt server ip:</label><input type='text' name='BROKER_URL' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, BROKER_URL);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_PORT'>mqtt server port:</label><input type='number' name='BROKER_PORT' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, broker_port_char);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_USER'>mqtt username:</label><input type='text' name='BROKER_USER' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, BROKER_USER);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='BROKER_PASSWORD'>mqtt password:</label><input type='text' name='BROKER_PASSWORD' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, BROKER_PASSWORD);

    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='MQTT_TOPIC'>subscribed mqtt topic:</label><input type='text' name='MQTT_TOPIC' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, MQTT_TOPIC);

    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='MQTT_LIGHTS_TOPICS'>comma separated lights topic:</label><input type='text' name='MQTT_LIGHTS_TOPICS' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, MQTT_LIGHTS_TOPICS);

    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='LIGHTS_INTENSITY_OPEN'>open door lights intensity:</label><input type='text' name='LIGHTS_INTENSITY_OPEN' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, LIGHTS_INTENSITY_OPEN);

    strcpy(resp_str_aux, (char * restrict)"'></div><div><label for='LIGHTS_INTENSITY_CLOSE'>closed door lights intensity:</label><input type='text' name='LIGHTS_INTENSITY_CLOSE' value='\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);
    strcat(resp_str, LIGHTS_INTENSITY_CLOSE);
    
    strcpy(resp_str_aux, (char * restrict)"'></div><div><input type='submit' value='Submit'></div></form>\0");
    if (sizeof(resp_str)/sizeof(char) < strlen(resp_str) + strlen(resp_str_aux))
        resp_str = (char*) realloc(resp_str, (strlen(resp_str) + strlen(resp_str_aux) + 100) * sizeof(char));
    strcat(resp_str, resp_str_aux);

    /*
    the web page:

    <meta charset='utf-8'>
    <style>
        label {display: inline-block; width: 150px; text-align: right;}
    </style>
    <form method='post'>
        <div>
            <label for='STA_WIFI_SSID'>wifi name:</label>
            <input type='text' name='STA_WIFI_SSID' value='"STA_WIFI_SSID"'>
        </div>
        <div>
            <label for='STA_WIFI_PASSWORD'>wifi password:</label>
            <input type='text' name='STA_WIFI_PASSWORD' value='"STA_WIFI_PASSWORD"'>
        </div>
        <div>
            <label for='BROKER_URL'>mqtt server ip:</label>
            <input type='text' name='BROKER_URL' value='"BROKER_URL"'>
        </div>
        <div>
            <label for='BROKER_PORT'>mqtt server port:</label>
            <input type='number' name='BROKER_PORT' value='"broker_port_char"'>
        </div>
        <div>
            <label for='BROKER_USER'>mqtt username:</label>
            <input type='text' name='BROKER_USER' value='"BROKER_USER"'>
        </div>
        <div>
            <label for='BROKER_PASSWORD'>mqtt password:</label>
            <input type='text' name='BROKER_PASSWORD' value='"BROKER_PASSWORD"'>
        </div>
        <div>
            <label for='MQTT_SUBSCRIBED_TOPIC'>subscribed mqtt topic:</label>
            <input type='text' name='MQTT_SUBSCRIBED_TOPIC' value='"MQTT_SUBSCRIBED_TOPIC"'>
        </div>
        <div>
            <input type='submit' value='Submit'>
        </div>
    </form>"
    */
    
    
    ESP_LOGI(TAG, "web: sending web page");

    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

httpd_uri_t configuration_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = configuration_handler,
    .user_ctx  = "Node Configuration"
};

httpd_uri_t configuration_post_uri = {
    .uri       = "/",
    .method    = HTTP_POST,
    .handler   = configuration_post_handler,
    .user_ctx  = "Node Configuration post"
};

#endif // _PANCHO_WEB_H_