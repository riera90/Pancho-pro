#ifndef _PANCHO_PWM_H_
#define _PANCHO_PWM_H_

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "driver/pwm.h"

#include "../../include/spiffs.h"


#define TAG CONFIG_TAG

// PWM period 1000us(1Khz), same as depth
#define PWM_PERIOD          (1000)

#define PWM_0_OUT_IO_NUM 15 // red
#define PWM_1_OUT_IO_NUM 12 // green
#define PWM_2_OUT_IO_NUM 13 // blue


const uint32_t pin_num[3] = {
    PWM_0_OUT_IO_NUM,
    PWM_1_OUT_IO_NUM,
    PWM_2_OUT_IO_NUM,
};

uint32_t duties[3] = {
    0, 0, 0
};

uint32_t new_duties[3] = {
    0, 0, 0
};

int16_t phase[3] = {
    0, 0, 0
};

void change_to_new_pwm_duties(void) {
    int32_t difference_per_cicle[3];

    for (size_t i = 0; i < 3; i++)
        difference_per_cicle[i] = (int32_t) (((int16_t)new_duties[i] - (int16_t)duties[i]) / 50);

    ESP_LOGI(TAG, "difference_per_cicle: r:%i g:%i b:%i", difference_per_cicle[0], difference_per_cicle[1], difference_per_cicle[2]);
    
    for (size_t i = 0; i < 50; i++)
    {
        for (size_t i = 0; i < 3; i++)
            duties[i] += difference_per_cicle[i];
        
        pwm_set_duties(duties);
        pwm_start(); // restart them with new configuration
        ESP_LOGI(TAG, "pwm duties: r:%u g:%u b:%u", duties[0], duties[1], duties[2]);
        vTaskDelay( (2000/50) / portTICK_RATE_MS);
    }
    
    for (size_t i = 0; i < 3; i++)
        duties[i] = new_duties[i];
    pwm_set_duties(duties);
    pwm_start(); // restart them with new configuration
}


#endif // _PANCHO_PWM_H_