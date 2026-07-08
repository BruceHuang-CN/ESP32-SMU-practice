#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <stdbool.h>

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#else
typedef int gpio_num_t;
#endif

bool pir_sensor_level_to_detected(int level);
void pir_sensor_init(gpio_num_t gpio);
bool pir_sensor_read(gpio_num_t gpio);

#endif
