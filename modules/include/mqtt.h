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

#define TAG CONFIG_TAG

char BROKER_URL[255];
int BROKER_PORT;
char BROKER_USER[255];
char BROKER_PASSWORD[255];

void command_handler(esp_mqtt_event_handle_t event);

void initialize_topic_subscriptions(esp_mqtt_client_handle_t client);

void mqtt_subscribe_to_topic(esp_mqtt_client_handle_t client, char* topic, int qos) {
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    ESP_LOGI(TAG, "mqtt: sent subscribe successful, msg_id=%d", msg_id);
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
    esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "mqtt: MQTT_EVENT_CONNECTED");
            initialize_topic_subscriptions(client);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "mqtt: MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "mqtt: MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "mqtt: MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "mqtt: MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "mqtt: MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            command_handler(event);
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "mqtt: MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static void mqtt_init(void) {
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
        
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = BROKER_URL,
        .port = BROKER_PORT,
        .username = BROKER_USER,
        .password = BROKER_PASSWORD,
        .event_handle = mqtt_event_handler
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

#endif // _PANCHO_MQTT_H_