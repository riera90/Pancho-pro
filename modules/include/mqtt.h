#ifndef _PANCHO_MQTT_H_
#define _PANCHO_MQTT_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"


void command_handler(esp_mqtt_event_handle_t event);

void initialize_topic_subscriptions(esp_mqtt_client_handle_t client);

void mqtt_subscribe_to_topic(esp_mqtt_client_handle_t client, char* topic, int qos) {
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    ESP_LOGI(CONFIG_TAG, "mqtt: sent subscribe successful, msg_id=%d", msg_id);
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
    esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(CONFIG_TAG, "mqtt: MQTT_EVENT_CONNECTED");
            initialize_topic_subscriptions(client);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(CONFIG_TAG, "mqtt: MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(CONFIG_TAG, "mqtt: MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(CONFIG_TAG, "mqtt: MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(CONFIG_TAG, "mqtt: MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(CONFIG_TAG, "mqtt: MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            command_handler(event);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(CONFIG_TAG, "mqtt: MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
        .port = CONFIG_BROKER_PORT,
        .username = CONFIG_BROKER_USER,
        .password = CONFIG_BROKER_PASSWORD,
        .event_handle = mqtt_event_handler,
        // .user_context = (void *)your_context
    };

#ifdef CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(CONFIG_TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

#endif