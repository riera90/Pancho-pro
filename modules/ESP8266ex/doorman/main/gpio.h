#ifndef _PANCHO_GPIO_H_
#define _PANCHO_GPIO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"

#include "../../include/mqtt.h"

#define TAG CONFIG_TAG

static xQueueHandle gpio_evt_queue = NULL;

static void gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, arg, NULL);
}


static void gpio_task(void* arg)
{
    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            // // uint16_t index;
            // // char* pch;
            // // char mqtt_lights_topics[16][256];
            // // char raw_mqtt_lights_topics[1024];
            // char light_intensity[32];


            // // strcpy(raw_mqtt_lights_topics, CONFIG_MQTT_LIGHTS_TOPICS);
            // // index = 0;
            // // pch = strtok(raw_mqtt_lights_topics, ",");
            // // while (raw_mqtt_lights_topics != NULL) {
            // //     strcpy(mqtt_lights_topics[index++], pch);
            // //     ESP_LOGW(TAG, "appending %s", pch);
            // //     pch = strtok(NULL, ",");
            // // }    

            // if (gpio_get_level(CONFIG_HALL_EFFECT_SENSOR_PIN)) { // door has been closed closed
            //     esp_mqtt_client_publish(mqtt_client, CONFIG_MQTT_TOPIC, "closed", strlen("closed"), CONFIG_BROKER_QOS, CONFIG_BROKER_RETAIN);
            //     strcpy(light_intensity, CONFIG_LIGHTS_INTENSITY_CLOSE);
            // }
            // else { // dor has been oppened
            //     esp_mqtt_client_publish(mqtt_client, CONFIG_MQTT_TOPIC, "oppened", strlen("oppened"), CONFIG_BROKER_QOS, CONFIG_BROKER_RETAIN);
            //     strcpy(light_intensity, CONFIG_LIGHTS_INTENSITY_OPEN);
            // }

            // ESP_LOGW(TAG, "light %s", light_intensity);
        }
    }
}


void init_gpio() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<CONFIG_HALL_EFFECT_SENSOR_PIN);
    io_conf.pull_down_en = 0x0;
    io_conf.pull_up_en = 0x1;
    gpio_config(&io_conf);
    
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

    gpio_install_isr_service(0); // useless parameter, only used in esp32
    gpio_isr_handler_add(CONFIG_HALL_EFFECT_SENSOR_PIN, gpio_isr_handler, (void*) CONFIG_HALL_EFFECT_SENSOR_PIN);
    ESP_LOGI(TAG, "gpio: hall effect gpio initialized");
}

#endif // _PANCHO_GPIO_H_