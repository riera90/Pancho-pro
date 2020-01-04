#ifndef _PANCHO_MQTT_HANDLER_H
#define _PANCHO_MQTT_HANDLER_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mqtt_client.h"

#include "../../include/spiffs.h"

#define TAG CONFIG_TAG

char MQTT_SUBSCRIBED_TOPIC[255];


void command_handler(esp_mqtt_event_handle_t event) {
    char* pch;
    size_t index = 0;
    char* buffer = (char*)calloc((event->data_len/sizeof(char)) + 1, sizeof(char));

    for (size_t i = 0; i < event->data_len/sizeof(char); i++) 
        buffer[i] = event->data[i];
    buffer[event->data_len/sizeof(char)] = '\0';
        
    
    pch = strtok(buffer, ",");
    
    while (pch != NULL && index < 3)
    {
        new_duties[index] = (uint32_t)atoi(pch);
        index++;
        pch = strtok (NULL, ",");
    }
    free(buffer);
    for (size_t i = 0; i < 3; i++) {
        if (new_duties[i] > PWM_PERIOD) {
            new_duties[i] = (uint32_t)PWM_PERIOD;
            ESP_LOGE(TAG, "PWM: the duty can't be more than the period");
        }
    }
    ESP_LOGI(TAG, "new pwm duties: r:%u g:%u g:%u", new_duties[0], new_duties[1], new_duties[2]);
    change_to_new_pwm_duties();
    ESP_LOGI(TAG, "pwm duties: r:%u g:%u g:%u", duties[0], duties[1], duties[2]);
}

void initialize_topic_subscriptions(esp_mqtt_client_handle_t client) {
    ESP_LOGI(TAG, "subscribint to %s", MQTT_SUBSCRIBED_TOPIC);
    mqtt_subscribe_to_topic(client, MQTT_SUBSCRIBED_TOPIC, CONFIG_BROKER_QOS);
}

#endif // _PANCHO_MQTT_HANDLER_H