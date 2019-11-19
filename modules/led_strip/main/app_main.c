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

#include "../../include/mqtt.h"
#include "../../include/wireless.h"


void command_handler(esp_mqtt_event_handle_t event) {

}

void initialize_topic_subscriptions(esp_mqtt_client_handle_t client) {
    mqtt_subscribe_to_topic(client, CONFIG_MQTT_SUBSCRIBED_TOPIC, CONFIG_BROKER_QOS);
}


void app_main() {
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
