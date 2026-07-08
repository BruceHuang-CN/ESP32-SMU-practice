#ifndef OBSTACLE_SENSOR_H
#define OBSTACLE_SENSOR_H

#include <stdbool.h>

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#else
typedef int gpio_num_t;
#endif

bool obstacle_sensor_level_to_detected(int level);
void obstacle_sensor_init(gpio_num_t gpio);
bool obstacle_sensor_read(gpio_num_t gpio);

#endif
