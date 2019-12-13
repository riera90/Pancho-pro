#ifndef _PANCHO_FACTORY_RESET_H_
#define _PANCHO_FACTORY_RESET_H_

#include <sys/unistd.h>
#include <sys/stat.h>
#include <string.h> 
#include "esp_err.h"
#include "esp_log.h"

#define TAG CONFIG_TAG

#ifdef GPIO_INPUT_PIN_SEL
#define GPIO_INPUT_PIN_SEL  (1ULL<<CONFIG_RESET_PIN) | GPIO_INPUT_PIN_SEL
#else
#define GPIO_INPUT_PIN_SEL  (1ULL<<CONFIG_RESET_PIN)
#endif


#endif // _PANCHO_FACTORY_RESET_H_