#ifndef _PANCHO_MQTT_HANDLER_H
#define _PANCHO_MQTT_HANDLER_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "mqtt_client.h"

#include "../../include/spiffs.h"
#include "gpio.h"

#define TAG CONFIG_TAG

char MQTT_TOPIC[255];
char MQTT_LIGHTS_TOPICS[1024];
char LIGHTS_INTENSITY_OPEN[255];
char LIGHTS_INTENSITY_CLOSE[255];

void mqtt_post_connection_phase(esp_mqtt_client_handle_t client) {
    init_gpio();
}

void command_handler(esp_mqtt_event_handle_t event) {
    return;
}

void initialize_topic_subscriptions(esp_mqtt_client_handle_t client) {
    return;
}

void set_all_lights_to(char* intensity) {
    return;
}

#endif // _PANCHO_MQTT_HANDLER_H