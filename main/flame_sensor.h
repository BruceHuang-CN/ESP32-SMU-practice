#ifndef FLAME_SENSOR_H
#define FLAME_SENSOR_H

#include <stdbool.h>

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#else
typedef int gpio_num_t;
#endif

bool flame_sensor_level_to_detected(int level);
void flame_sensor_init(gpio_num_t gpio);
bool flame_sensor_read(gpio_num_t gpio);

#endif
