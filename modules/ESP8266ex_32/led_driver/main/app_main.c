#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "../../include/mqtt.h"
#include "../../include/wireless.h"
#include "../../include/spiffs.h"
#include "../../include/web_server.h"
#include "../../include/reset_and_configuration.h"

#include "pwm.h"
#include "configuration.h"
#include "web.h"
#include "mqtt_handler.h"

#define TAG CONFIG_TAG

void app_main(void) {
    pwm_init(PWM_PERIOD, duties, 3, pin_num);
    pwm_set_phases(phase);
    pwm_start();
    reset_and_config_gpio_init();

    ESP_LOGI(TAG, "Startup..");
    ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    if (check_gpio_for_factory_reset())
        reset_node_configuration();

    if (!load_node_configuration() || check_gpio_for_configuration_mode()) {
        ESP_LOGI(TAG, "entering softap mode");
        wifi_init_softap();
        httpd_uri_t* uris[2] = {&configuration_uri, &configuration_post_uri};
        config_web_server_init(uris, 2);
    } else {
        ESP_LOGI(TAG, "entering sta mode");
        wifi_init_sta();
        mqtt_init();
    }
}