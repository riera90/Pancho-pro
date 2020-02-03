#ifndef _PANCHO_RESET_AND_CONFIGURATION_H_
#define _PANCHO_RESET_AND_CONFIGURATION_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "driver/gpio.h"

#include "spiffs.h"

#define TAG CONFIG_TAG

// gpio pin sel for the factory reset button and the configuration switch
#define GPIO_INPUT_PIN_SEL  (1ULL<<CONFIG_CONFIGURATION_MODE_PIN) | (1ULL<<CONFIG_FACTORY_RESET_PIN)

void reset_and_config_gpio_init(void) {
    
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    ESP_LOGI(TAG, "gpio: gpio initialized");
}

bool check_gpio_for_factory_reset(void) {
    if (gpio_get_level(CONFIG_FACTORY_RESET_PIN) == 1)
        return false;

    vTaskDelay(1000 / portTICK_RATE_MS);

    if (gpio_get_level(CONFIG_FACTORY_RESET_PIN) == 1)
        return false;
        
    ESP_LOGI(TAG, "Entering factory reset");

    return true;
}

bool check_gpio_for_configuration_mode(void) {
    if (gpio_get_level(CONFIG_CONFIGURATION_MODE_PIN) == 1)
        return false;

    vTaskDelay(1000 / portTICK_RATE_MS);

    if (gpio_get_level(CONFIG_CONFIGURATION_MODE_PIN) == 1)
        return false;
        
    ESP_LOGI(TAG, "Entering configuration mode forced by user");

    return true;
}


#endif // _PANCHO_RESET_AND_CONfIGURATION_H_