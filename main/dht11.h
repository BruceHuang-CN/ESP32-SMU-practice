#ifndef DHT11_H
#define DHT11_H

#include <stdbool.h>

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#else
typedef int gpio_num_t;
#endif

typedef struct {
    float temperature_c;
    float humidity_percent;
} dht11_reading_t;

bool dht11_parse_bytes(const unsigned char bytes[5], float *temperature_c, float *humidity_percent);
void dht11_init(gpio_num_t gpio);
bool dht11_read(gpio_num_t gpio, dht11_reading_t *reading);

#endif
