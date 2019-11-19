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

#include "driver/pwm.h"

#include "../../include/mqtt.h"
#include "../../include/wireless.h"


#define PWM_0_OUT_IO_NUM 15 // red
#define PWM_1_OUT_IO_NUM 12 // green
#define PWM_2_OUT_IO_NUM 13 // blue

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD    (1000)

const uint32_t pin_num[3] = {
    PWM_0_OUT_IO_NUM,
    PWM_1_OUT_IO_NUM,
    PWM_2_OUT_IO_NUM,
};

uint32_t duties[3] = {
    0, 0, 0
};

uint32_t new_duties[3] = {0, 0, 0};

int16_t phase[3] = {
    0, 0, 0
};

void change_to_new_pwm_duties() {
    int32_t difference_per_cicle[3];
    for (size_t i = 0; i < 3; i++)
        difference_per_cicle[i] = (int32_t) (((float)new_duties[i] - (float)duties[i]) / 50);

    ESP_LOGI(CONFIG_TAG, "difference_per_cicle: r:%i g:%i g:%i", difference_per_cicle[0], difference_per_cicle[1], difference_per_cicle[2]);
    
    for (size_t i = 0; i < 50; i++)
    {
        for (size_t i = 0; i < 3; i++)
            duties[i] += difference_per_cicle[i];
        
        pwm_set_duties(duties);
        pwm_start(); // restart them with new configuration
        ESP_LOGI(CONFIG_TAG, "pwm duties: r:%u g:%u g:%u", duties[0], duties[1], duties[2]);
        vTaskDelay( (2000/50) / portTICK_RATE_MS);
    }
    
    for (size_t i = 0; i < 3; i++)
        duties[i] = new_duties[i];
    pwm_set_duties(duties);
    pwm_start(); // restart them with new configuration
}

void command_handler(esp_mqtt_event_handle_t event) {
    
    char* buffer = (char*)calloc((event->data_len/sizeof(char)) + 1, sizeof(char));

    for (size_t i = 0; i < event->data_len/sizeof(char); i++) 
        buffer[i] = event->data[i];
    buffer[event->data_len/sizeof(char)] = '\0';
        
    char * pch;
    pch = strtok(buffer, ",");
    size_t index = 0;
    while (pch != NULL && index < 3)
    {
        new_duties[index] = (uint32_t)atoi(pch);
        index++;
        pch = strtok (NULL, ",");
    }
    free(buffer);
    for (size_t i = 0; i < 3; i++) {
        if (new_duties[i] > 1000) {
            new_duties[i] = (uint32_t)1000;
            ESP_LOGE(CONFIG_TAG, "PWM: the duty can't be more than the period");
        }
    }
    ESP_LOGI(CONFIG_TAG, "new pwm duties: r:%u g:%u g:%u", new_duties[0], new_duties[1], new_duties[2]);
    change_to_new_pwm_duties();
    ESP_LOGI(CONFIG_TAG, "pwm duties: r:%u g:%u g:%u", duties[0], duties[1], duties[2]);
}

void initialize_topic_subscriptions(esp_mqtt_client_handle_t client) {
    mqtt_subscribe_to_topic(client, CONFIG_MQTT_SUBSCRIBED_TOPIC, CONFIG_BROKER_QOS);
}


void app_main() {
    pwm_init(PWM_PERIOD, duties, 3, pin_num);
    pwm_set_phases(phase);
    pwm_start();

    ESP_LOGI(CONFIG_TAG, "Startup..");
    ESP_LOGI(CONFIG_TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(CONFIG_TAG, "IDF version: %s", esp_get_idf_version());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize wireless connection
    ESP_LOGI(CONFIG_TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // Initialize the MQTT connection
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    mqtt_app_start();
}
